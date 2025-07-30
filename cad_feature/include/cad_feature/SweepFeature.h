#pragma once

#include "Feature.h"
#include "cad_sketch/Sketch.h"
#include <vector>
#include <gp_Pln.hxx>

namespace cad_feature {

class SweepFeature : public Feature {
public:
    SweepFeature();
    SweepFeature(const std::string& name);
    virtual ~SweepFeature() = default;

    // Profile and path operations
    void SetProfile(const cad_sketch::SketchPtr& profile);
    const cad_sketch::SketchPtr& GetProfile() const;
    
    void SetPath(const cad_sketch::SketchPtr& path);
    const cad_sketch::SketchPtr& GetPath() const;

    void SetProfilePlane(const gp_Pln& plane);
    void SetPathPlane(const gp_Pln& plane);

    
    // Sweep parameters
    void SetTwistAngle(double angle);
    double GetTwistAngle() const;
    
    void SetScaleFactor(double factor);
    double GetScaleFactor() const;
    
    void SetKeepOriginalOrientation(bool keep);
    bool GetKeepOriginalOrientation() const;
    
    // Feature interface
    cad_core::ShapePtr CreateShape() const override;
    bool ValidateParameters() const override;
    std::shared_ptr<cad_core::ICommand> CreateCommand() const override;

private:
    cad_sketch::SketchPtr m_profile;
    cad_sketch::SketchPtr m_path;

    gp_Pln m_profilePlane;
    gp_Pln m_pathPlane;
    
    bool IsProfileValid() const;
    bool IsPathValid() const;
    cad_core::ShapePtr SweepProfile() const;
};

using SweepFeaturePtr = std::shared_ptr<SweepFeature>;

} // namespace cad_feature