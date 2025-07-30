#include "cad_feature/SweepFeature.h"
#include "cad_core/CreateBoxCommand.h"
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <Geom_Circle.hxx>
#include <gp_Ax2.hxx>
#include <cmath>
#include <ElSLib.hxx>

namespace cad_feature {

// 将草图对象转换为OpenCASCADE的线框
TopoDS_Wire ConvertSketchToWire(const cad_sketch::SketchPtr& sketch, const gp_Pln& plane) {
    if (!sketch || sketch->IsEmpty()) {
        return TopoDS_Wire();
    }

    BRepBuilderAPI_MakeWire wireMaker;
    for (const auto& elem : sketch->GetElements()) {
        if (elem->GetType() == cad_sketch::SketchElementType::Line) {
            auto line = std::static_pointer_cast<cad_sketch::SketchLine>(elem);
            const auto& p1_2d = line->GetStartPoint()->GetPoint().GetOCCTPoint();
            const auto& p2_2d = line->GetEndPoint()->GetPoint().GetOCCTPoint();

            //  使用 ElSLib::Value 将2D点转换为3D平面上的点
            gp_Pnt p1_3d = ElSLib::Value(p1_2d.X(), p1_2d.Y(), plane);
            gp_Pnt p2_3d = ElSLib::Value(p2_2d.X(), p2_2d.Y(), plane);

            if (!p1_3d.IsEqual(p2_3d, 1e-7)) {
                wireMaker.Add(BRepBuilderAPI_MakeEdge(p1_3d, p2_3d).Edge());
            }
        }
        else if (elem->GetType() == cad_sketch::SketchElementType::Circle) {
            auto circle = std::static_pointer_cast<cad_sketch::SketchCircle>(elem);
            const auto& center_2d = circle->GetCenter()->GetPoint().GetOCCTPoint();

            //  同样转换圆心，并使用平面的法线作为圆的轴向
            gp_Pnt center_3d = ElSLib::Value(center_2d.X(), center_2d.Y(), plane);
            gp_Ax2 axis(center_3d, plane.Axis().Direction()); // 使用平面的法线

            Handle(Geom_Circle) geomCircle = new Geom_Circle(axis, circle->GetRadius());
            wireMaker.Add(BRepBuilderAPI_MakeEdge(geomCircle).Edge());
        }
    }
    return wireMaker.Wire();
}


SweepFeature::SweepFeature() : Feature(FeatureType::Sweep, "Sweep") {
    SetParameter("twist_angle", 0.0);
    SetParameter("scale_factor", 1.0);
    SetParameter("keep_orientation", 1.0);
}

SweepFeature::SweepFeature(const std::string& name) : Feature(FeatureType::Sweep, name) {
    SetParameter("twist_angle", 0.0);
    SetParameter("scale_factor", 1.0);
    SetParameter("keep_orientation", 1.0);
}

void SweepFeature::SetProfilePlane(const gp_Pln& plane) {
    m_profilePlane = plane;
}

void SweepFeature::SetPathPlane(const gp_Pln& plane) {
    m_pathPlane = plane;
}

void SweepFeature::SetProfile(const cad_sketch::SketchPtr& profile) {
    m_profile = profile;
}

const cad_sketch::SketchPtr& SweepFeature::GetProfile() const {
    return m_profile;
}

void SweepFeature::SetPath(const cad_sketch::SketchPtr& path) {
    m_path = path;
}

const cad_sketch::SketchPtr& SweepFeature::GetPath() const {
    return m_path;
}

void SweepFeature::SetTwistAngle(double angle) {
    SetParameter("twist_angle", angle);
}

double SweepFeature::GetTwistAngle() const {
    return GetParameter("twist_angle");
}

void SweepFeature::SetScaleFactor(double factor) {
    SetParameter("scale_factor", factor);
}

double SweepFeature::GetScaleFactor() const {
    return GetParameter("scale_factor");
}

void SweepFeature::SetKeepOriginalOrientation(bool keep) {
    SetParameter("keep_orientation", keep ? 1.0 : 0.0);
}

bool SweepFeature::GetKeepOriginalOrientation() const {
    return GetParameter("keep_orientation") != 0.0;
}


//核心逻辑：创建扫掠
cad_core::ShapePtr SweepFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return SweepProfile();
}

bool SweepFeature::ValidateParameters() const {
    if (!IsProfileValid() || !IsPathValid()) {
        return false;
    }
    
    double scaleFactor = GetScaleFactor();
    if (scaleFactor <= 0.0) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> SweepFeature::CreateCommand() const {
    // For now, return a simple box command as placeholder
    return std::make_shared<cad_core::CreateBoxCommand>(10.0, 10.0, 10.0);
}

bool SweepFeature::IsProfileValid() const {
    return m_profile && !m_profile->IsEmpty();
}

bool SweepFeature::IsPathValid() const {
    return m_path && !m_path->IsEmpty();
}

cad_core::ShapePtr SweepFeature::SweepProfile() const {
    if (!IsProfileValid() || !IsPathValid()) {
        return nullptr;
    }
    
    try {
        // 1. 将草图转换为 OpenCASCADE 的线框
        TopoDS_Wire profileWire = ConvertSketchToWire(m_profile, m_profilePlane);
        TopoDS_Wire pathWire = ConvertSketchToWire(m_path, m_pathPlane);

        if (profileWire.IsNull() || pathWire.IsNull()) {
            return nullptr;
        }

        // 2. 使用核心API BRepOffsetAPI_MakePipe 执行扫掠
        BRepOffsetAPI_MakePipe pipeMaker(pathWire, profileWire);
        pipeMaker.Build();

        // 3. 检查并返回结果
        if (pipeMaker.IsDone()) {
            return std::make_shared<cad_core::Shape>(pipeMaker.Shape());
        }
    } catch (const Standard_Failure& e) {
        //异常处理
        return nullptr;
    }

	return nullptr;
}

} // namespace cad_feature