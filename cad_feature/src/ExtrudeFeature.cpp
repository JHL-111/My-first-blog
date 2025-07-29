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
    if (distance <= 0.0) {
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

        // 处理所有草图元素
        for (const auto& elem : elements) {
            if (elem->GetType() == cad_sketch::SketchElementType::Line) {
                auto sketchLine = std::static_pointer_cast<cad_sketch::SketchLine>(elem);
                const auto& startPnt = sketchLine->GetStartPoint()->GetPoint().GetOCCTPoint();
                const auto& endPnt = sketchLine->GetEndPoint()->GetPoint().GetOCCTPoint();
                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(startPnt, endPnt).Edge();
                wireMaker.Add(edge);
            }
            else if (elem->GetType() == cad_sketch::SketchElementType::Circle) {
                // 添加圆形处理
                auto sketchCircle = std::static_pointer_cast<cad_sketch::SketchCircle>(elem);
                const auto& center = sketchCircle->GetCenter()->GetPoint().GetOCCTPoint();
                double radius = sketchCircle->GetRadius();

                // 创建圆形边
                gp_Ax2 axis(center, gp_Dir(0, 0, 1)); // 假设在XY平面
                Handle(Geom_Circle) geomCircle = new Geom_Circle(axis, radius);
                TopoDS_Edge circleEdge = BRepBuilderAPI_MakeEdge(geomCircle).Edge();
                wireMaker.Add(circleEdge);
            }
        }

        // 对于单个圆，直接创建面
        if (elements.size() == 1 && elements[0]->GetType() == cad_sketch::SketchElementType::Circle) {
            auto sketchCircle = std::static_pointer_cast<cad_sketch::SketchCircle>(elements[0]);
            const auto& center = sketchCircle->GetCenter()->GetPoint().GetOCCTPoint();
            double radius = sketchCircle->GetRadius();

            // 创建圆形线框
            gp_Ax2 axis(center, gp_Dir(0, 0, 1));
            Handle(Geom_Circle) geomCircle = new Geom_Circle(axis, radius);
            TopoDS_Edge circleEdge = BRepBuilderAPI_MakeEdge(geomCircle).Edge();
            TopoDS_Wire circleWire = BRepBuilderAPI_MakeWire(circleEdge).Wire();

            // 创建圆形面
            TopoDS_Face circleFace = BRepBuilderAPI_MakeFace(circleWire).Face();

            // 执行拉伸
            double distance = GetDistance();
            gp_Vec extrudeVector(0, 0, distance);
            BRepPrimAPI_MakePrism prismMaker(circleFace, extrudeVector);
            prismMaker.Build();

            if (prismMaker.IsDone()) {
                return std::make_shared<cad_core::Shape>(prismMaker.Shape());
            }
        }
        else {
            // 处理多个元素组成的轮廓
            TopoDS_Wire sketchWire = wireMaker.Wire();
            if (sketchWire.IsNull()) {
                return nullptr;
            }

            TopoDS_Face sketchFace = BRepBuilderAPI_MakeFace(sketchWire).Face();
            if (sketchFace.IsNull()) {
                return nullptr;
            }

            double distance = GetDistance();
            gp_Vec extrudeVector(0, 0, distance);
            BRepPrimAPI_MakePrism prismMaker(sketchFace, extrudeVector);
            prismMaker.Build();

            if (prismMaker.IsDone()) {
                return std::make_shared<cad_core::Shape>(prismMaker.Shape());
            }
        }
    }
    catch (const Standard_Failure& e) {
        return nullptr;
    }

    return nullptr;
}

} // namespace cad_feature