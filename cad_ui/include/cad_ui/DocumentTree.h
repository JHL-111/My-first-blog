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
    // ����ɾ��һ�������� (Shape)
    void shapeDeleteRequested(const cad_core::ShapePtr& shape);
    // ����ɾ��һ������ (Feature)
    void featureDeleteRequested(const cad_feature::FeaturePtr& feature);
    // ����ɾ��һ����ͼ (Sketch)
    void sketchDeleteRequested(const cad_sketch::SketchPtr& sketch);
	// ��������һ�������� (Shape)
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

// ��������ֻ��������ָ�룬����Ҫ�����������ඨ�壬
// ֻ��Ҫ��ʹ��ǰ���߱�������Щ���ʹ��ڼ��ɡ�
// �������Լӿ�����ٶȲ�����������
Q_DECLARE_METATYPE(cad_core::ShapePtr)
Q_DECLARE_METATYPE(cad_feature::FeaturePtr)
Q_DECLARE_METATYPE(cad_sketch::SketchPtr)