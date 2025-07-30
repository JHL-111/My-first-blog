#include "cad_feature/LoftFeature.h"
#include "cad_core/CreateSphereCommand.h"
#include <BRepOffsetAPI_ThruSections.hxx>
#include <algorithm>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Edge.hxx>
#include <ElSLib.hxx>
#include "cad_sketch/SketchLine.h"
#include "cad_sketch/SketchCircle.h"
#include <Geom_Circle.hxx>
#include <gp_Ax2.hxx>


namespace cad_feature {

TopoDS_Wire ConvertSketchToWire(const cad_sketch::SketchPtr& sketch) {
    if (!sketch || sketch->IsEmpty()) {
        return TopoDS_Wire();
    }

    BRepBuilderAPI_MakeWire wireMaker;
    const auto& plane = sketch->GetPlane(); // 获取草图所在的3D平面

    for (const auto& elem : sketch->GetElements()) {
        if (elem->GetType() == cad_sketch::SketchElementType::Line) {
            auto line = std::static_pointer_cast<cad_sketch::SketchLine>(elem);
            const auto& p1_2d = line->GetStartPoint()->GetPoint().GetOCCTPoint();
            const auto& p2_2d = line->GetEndPoint()->GetPoint().GetOCCTPoint();

            // 使用 ElSLib::Value 将2D点转换为3D平面上的点
            gp_Pnt p1_3d = ElSLib::Value(p1_2d.X(), p1_2d.Y(), plane);
            gp_Pnt p2_3d = ElSLib::Value(p2_2d.X(), p2_2d.Y(), plane);

            if (!p1_3d.IsEqual(p2_3d, 1e-7)) {
                wireMaker.Add(BRepBuilderAPI_MakeEdge(p1_3d, p2_3d).Edge());
            }
        }
        else if (elem->GetType() == cad_sketch::SketchElementType::Circle) {
            auto circle = std::static_pointer_cast<cad_sketch::SketchCircle>(elem);
            const auto& center_2d = circle->GetCenter()->GetPoint().GetOCCTPoint();
            double radius = circle->GetRadius();

            // 同样转换圆心，并使用平面的法线作为圆的轴向
            gp_Pnt center_3d = ElSLib::Value(center_2d.X(), center_2d.Y(), plane);
            gp_Ax2 axis(center_3d, plane.Axis().Direction());

            Handle(Geom_Circle) geomCircle = new Geom_Circle(axis, radius);
            wireMaker.Add(BRepBuilderAPI_MakeEdge(geomCircle).Edge());
        }
        // 您可以在这里添加对其他草图元素（如圆弧）的支持
    }
    return wireMaker.Wire();
}
    
LoftFeature::LoftFeature() : Feature(FeatureType::Loft, "Loft") {
    SetParameter("solid", 1.0);
    SetParameter("ruled", 0.0);
    SetParameter("closed", 0.0);
}

LoftFeature::LoftFeature(const std::string& name) : Feature(FeatureType::Loft, name) {
    SetParameter("solid", 1.0);
    SetParameter("ruled", 0.0);
    SetParameter("closed", 0.0);
}

void LoftFeature::AddSection(const cad_sketch::SketchPtr& section) {
    m_sections.push_back(section);
}

void LoftFeature::RemoveSection(const cad_sketch::SketchPtr& section) {
    auto it = std::find(m_sections.begin(), m_sections.end(), section);
    if (it != m_sections.end()) {
        m_sections.erase(it);
    }
}

void LoftFeature::ClearSections() {
    m_sections.clear();
}

const std::vector<cad_sketch::SketchPtr>& LoftFeature::GetSections() const {
    return m_sections;
}

int LoftFeature::GetSectionCount() const {
    return static_cast<int>(m_sections.size());
}

void LoftFeature::AddGuideCurve(const cad_sketch::SketchPtr& guide) {
    m_guideCurves.push_back(guide);
}

void LoftFeature::RemoveGuideCurve(const cad_sketch::SketchPtr& guide) {
    auto it = std::find(m_guideCurves.begin(), m_guideCurves.end(), guide);
    if (it != m_guideCurves.end()) {
        m_guideCurves.erase(it);
    }
}

void LoftFeature::ClearGuideCurves() {
    m_guideCurves.clear();
}

const std::vector<cad_sketch::SketchPtr>& LoftFeature::GetGuideCurves() const {
    return m_guideCurves;
}

int LoftFeature::GetGuideCurveCount() const {
    return static_cast<int>(m_guideCurves.size());
}

void LoftFeature::SetSolid(bool solid) {
    SetParameter("solid", solid ? 1.0 : 0.0);
}

bool LoftFeature::GetSolid() const {
    return GetParameter("solid") != 0.0;
}

void LoftFeature::SetRuled(bool ruled) {
    SetParameter("ruled", ruled ? 1.0 : 0.0);
}

bool LoftFeature::GetRuled() const {
    return GetParameter("ruled") != 0.0;
}

void LoftFeature::SetClosed(bool closed) {
    SetParameter("closed", closed ? 1.0 : 0.0);
}

bool LoftFeature::GetClosed() const {
    return GetParameter("closed") != 0.0;
}

cad_core::ShapePtr LoftFeature::CreateShape() const {
    if (!ValidateParameters()) {
        return nullptr;
    }
    
    return LoftSections();
}

bool LoftFeature::ValidateParameters() const {
    if (!AreSectionsValid()) {
        return false;
    }
    
    if (GetSectionCount() < 2) {
        return false;
    }
    
    return true;
}

std::shared_ptr<cad_core::ICommand> LoftFeature::CreateCommand() const {
    // For now, return a simple sphere command as placeholder
    return std::make_shared<cad_core::CreateSphereCommand>(5.0);
}

bool LoftFeature::AreSectionsValid() const {
    for (const auto& section : m_sections) {
        if (!section || section->IsEmpty()) {
            return false;
        }
    }
    return true;
}

bool LoftFeature::AreGuideCurvesValid() const {
    for (const auto& guide : m_guideCurves) {
        if (!guide || guide->IsEmpty()) {
            return false;
        }
    }
    return true;
}

cad_core::ShapePtr LoftFeature::LoftSections() const {
    if (!AreSectionsValid() || GetSectionCount() < 2) {
        return nullptr;
    }

    try {
        // 1. 初始化放样工具，并设置参数
        BRepOffsetAPI_ThruSections thruSections(GetSolid(), GetRuled(), 1.0e-6);

        // 2. 将所有截面草图转换为OCCT线框并添加到放样工具中
        for (const auto& sectionSketch : m_sections) {
            TopoDS_Wire wire = ConvertSketchToWire(sectionSketch);
            if (!wire.IsNull()) {
                thruSections.AddWire(wire);
            }
            else {
                // 如果有任何一个截面无效，则放样失败
                return nullptr;
            }
        }

        // 3. 执行放样计算
        thruSections.Build();

        // 4. 检查计算是否成功，并返回结果
        if (thruSections.IsDone()) {
            return std::make_shared<cad_core::Shape>(thruSections.Shape());
        }
    }
    catch (const Standard_Failure& e) {
        // 捕获并处理OpenCASCADE的异常
        return nullptr;
    }

    return nullptr;
}

} // namespace cad_feature