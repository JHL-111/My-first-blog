#include "cad_feature/ExtrudeFeature.h"
#include "cad_core/CreateBoxCommand.h"
#include <BRepPrimAPI_MakePrism.hxx>
#include <gp_Vec.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#pragma execution_character_set("utf-8")
#include <Geom_Circle.hxx>
#include <gp_Ax2.hxx>
#include <gp_Trsf.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include<cmath>
#include <gp_Pnt.hxx>  
#include <gp_Dir.hxx>  
#include <gp_Ax3.hxx> 
#include <TopoDS_Face.hxx>
#include <ElSLib.hxx>
#include <qdebug.h>

namespace cad_feature {

ExtrudeFeature::ExtrudeFeature() : Feature(FeatureType::Extrude, "Extrude") {
    SetParameter("distance", 10.0);
    SetParameter("direction_x", 0.0);
    SetParameter("direction_y", 0.0);
    SetParameter("direction_z", 1.0);
    SetParameter("taper_angle", 0.0);
    SetParameter("midplane", 0.0);
}

ExtrudeFeature::ExtrudeFeature(const std::string& name) : Feature(FeatureType::Extrude, name) {
    SetParameter("distance", 10.0);
    SetParameter("direction_x", 0.0);
    SetParameter("direction_y", 0.0);
    SetParameter("direction_z", 1.0);
    SetParameter("taper_angle", 0.0);
    SetParameter("midplane", 0.0);
}

void ExtrudeFeature::SetSketch(const cad_sketch::SketchPtr& sketch) {
    m_sketch = sketch;
}

const cad_sketch::SketchPtr& ExtrudeFeature::GetSketch() const {
    return m_sketch;
}

void ExtrudeFeature::SetDistance(double distance) {
    SetParameter("distance", distance);
}

double ExtrudeFeature::GetDistance() const {
    return GetParameter("distance");
}

void ExtrudeFeature::SetDirection(double x, double y, double z) {
    SetParameter("direction_x", x);
    SetParameter("direction_y", y);
    SetParameter("direction_z", z);
}

void ExtrudeFeature::GetDirection(double& x, double& y, double& z) const {
    x = GetParameter("direction_x");
    y = GetParameter("direction_y");
    z = GetParameter("direction_z");
}

void ExtrudeFeature::SetTaperAngle(double angle) {
    SetParameter("taper_angle", angle);
}

double ExtrudeFeature::GetTaperAngle() const {
    return GetParameter("taper_angle");
}

void ExtrudeFeature::SetMidplane(bool midplane) {
    SetParameter("midplane", midplane ? 1.0 : 0.0);
}

bool ExtrudeFeature::GetMidplane() const {
    return GetParameter("midplane") != 0.0;
}

void ExtrudeFeature::SetSketchPlane(const gp_Pln& plane) {
    m_sketchPlane = plane;
}

cad_core::ShapePtr ExtrudeFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return ExtrudeSketch();
}

bool ExtrudeFeature::ValidateParameters() const {
    if (!IsSketchValid()) {
        return false;
    }
    
    double distance = GetDistance();
    if (std::abs(distance) < 1e-9) { // 检查距离是否接近于零
        return false;
    }
    
    double dx, dy, dz;
    GetDirection(dx, dy, dz);
    double length = std::sqrt(dx*dx + dy*dy + dz*dz);
    if (length < 1e-10) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> ExtrudeFeature::CreateCommand() const {
    // For now, return a simple box command as placeholder
    return std::make_shared<cad_core::CreateBoxCommand>(GetDistance(), GetDistance(), GetDistance());
}

bool ExtrudeFeature::IsSketchValid() const {
    return m_sketch && !m_sketch->IsEmpty();
}

cad_core::ShapePtr ExtrudeFeature::ExtrudeSketch() const {
    if (!IsSketchValid()) {
        return nullptr;
    }

    try {
        BRepBuilderAPI_MakeWire wireMaker;
        const auto& elements = m_sketch->GetElements();

        // 遍历所有草图元素，将它们的2D坐标转换为3D坐标并创建3D边
        for (const auto& elem : elements) {
            if (elem->GetType() == cad_sketch::SketchElementType::Line) {
                auto sketchLine = std::static_pointer_cast<cad_sketch::SketchLine>(elem);
                const auto& p1_2d = sketchLine->GetStartPoint()->GetPoint().GetOCCTPoint();
                const auto& p2_2d = sketchLine->GetEndPoint()->GetPoint().GetOCCTPoint();

                // 使用 ElSLib::Value 将2D点转换为草图平面上的3D点
                gp_Pnt p1_3d = ElSLib::Value(p1_2d.X(), p1_2d.Y(), m_sketchPlane);
                gp_Pnt p2_3d = ElSLib::Value(p2_2d.X(), p2_2d.Y(), m_sketchPlane);

                if (!p1_3d.IsEqual(p2_3d, 1e-7)) {
                    wireMaker.Add(BRepBuilderAPI_MakeEdge(p1_3d, p2_3d).Edge());
                }

            }
            else if (elem->GetType() == cad_sketch::SketchElementType::Circle) {
                auto sketchCircle = std::static_pointer_cast<cad_sketch::SketchCircle>(elem);
                const auto& center_2d = sketchCircle->GetCenter()->GetPoint().GetOCCTPoint();
                double radius = sketchCircle->GetRadius();

                // 将圆心从2D转换为3D
                gp_Pnt center_3d = ElSLib::Value(center_2d.X(), center_2d.Y(), m_sketchPlane);

                // 使用草图平面的坐标系创建圆
                gp_Ax2 circle_axis(center_3d, m_sketchPlane.Axis().Direction());
                Handle(Geom_Circle) geomCircle = new Geom_Circle(circle_axis, radius);
                wireMaker.Add(BRepBuilderAPI_MakeEdge(geomCircle).Edge());
            }
        }

        TopoDS_Wire sketchWire = wireMaker.Wire();
        if (sketchWire.IsNull() || sketchWire.Closed() == Standard_False) {
            // 如果线框无效或未闭合，则无法创建实体
            return nullptr;
        }

        // 从3D线框创建面，这个面现在位于正确的3D位置和方向
        TopoDS_Face sketchFace = BRepBuilderAPI_MakeFace(sketchWire).Face();
        if (sketchFace.IsNull()) {
            return nullptr;
        }

        // 计算拉伸向量，方向应为草图平面的法线方向
        double distance = GetDistance();
        gp_Dir extrude_dir = m_sketchPlane.Axis().Direction();
        gp_Vec extrudeVector = gp_Vec(extrude_dir) * distance;

        // 执行拉伸
        BRepPrimAPI_MakePrism prismMaker(sketchFace, extrudeVector);
        prismMaker.Build();

        if (prismMaker.IsDone()) {
            return std::make_shared<cad_core::Shape>(prismMaker.Shape());
        }

    }
    catch (const Standard_Failure& e) {
        // 捕获并处理OpenCASCADE的异常
        qDebug() << "Extrusion failed: " << e.GetMessageString();
        return nullptr;
    }

    return nullptr;
}

} // namespace cad_feature