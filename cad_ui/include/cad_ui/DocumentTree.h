#pragma once

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include "cad_core/Shape.h"
#include "cad_feature/Feature.h"
#include "cad_sketch/Sketch.h"

namespace cad_ui {

class DocumentTree : public QTreeWidget {
    Q_OBJECT

public:
    explicit DocumentTree(QWidget* parent = nullptr);
    ~DocumentTree() = default;

    void AddShape(const cad_core::ShapePtr& shape);
    void RemoveShape(const cad_core::ShapePtr& shape);
    void AddFeature(const cad_feature::FeaturePtr& feature);
    void RemoveFeature(const cad_feature::FeaturePtr& feature);
    void AddSketch(const cad_sketch::SketchPtr& sketch);
    void RemoveSketch(const cad_sketch::SketchPtr& sketch);
    int GetSketchCount() const;
    void Clear();
    void setItemVisibilityState(const QVariant& itemData, bool visible);

signals:
    void ShapeSelected(const cad_core::ShapePtr& shape);
    void FeatureSelected(const cad_feature::FeaturePtr& feature);
    void SketchSelected(const cad_sketch::SketchPtr& sketch);
    // 请求删除一个几何体 (Shape)
    void shapeDeleteRequested(const cad_core::ShapePtr& shape);
    // 请求删除一个特征 (Feature)
    void featureDeleteRequested(const cad_feature::FeaturePtr& feature);
    // 请求删除一个草图 (Sketch)
    void sketchDeleteRequested(const cad_sketch::SketchPtr& sketch);
	// 请求隐藏一个几何体 (Shape)
    void visibilityToggled(const QVariant& itemData);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void OnItemClicked(QTreeWidgetItem* item, int column);
    void OnItemDoubleClicked(QTreeWidgetItem* item, int column);
    void OnDeleteItem();
    void OnRenameItem();
    void OnToggleVisibility();

private:
    QTreeWidgetItem* m_shapesRoot;
    QTreeWidgetItem* m_featuresRoot;
    QMenu* m_contextMenu;
    QAction* m_deleteAction;
    QAction* m_renameAction;
    QAction* m_toggleVisibilityAction;
    QTreeWidgetItem* m_sketchesRoot;

    void CreateContextMenu();
    void SetupTree();
};

} // namespace cad_ui

#include <QMetaType>

// 由于我们只是用智能指针，不需要包含完整的类定义，
// 只需要在使用前告诉编译器这些类型存在即可。
// 这样可以加快编译速度并减少依赖。
Q_DECLARE_METATYPE(cad_core::ShapePtr)
Q_DECLARE_METATYPE(cad_feature::FeaturePtr)
Q_DECLARE_METATYPE(cad_sketch::SketchPtr)