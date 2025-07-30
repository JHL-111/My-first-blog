#include "cad_ui/LoftFeatureDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QMessageBox>

#pragma execution_character_set("utf-8")

namespace cad_ui {

LoftFeatureDialog::LoftFeatureDialog(QWidget* parent)
    : QDialog(parent) {
    setupUI();
    setModal(false);
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
}

void LoftFeatureDialog::setupUI() {
    setWindowTitle("创建放样特征");
    resize(400, 350);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(10);

    QGroupBox* selectionGroup = new QGroupBox("截面轮廓 (Profiles)", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(selectionGroup);

    m_profileListWidget = new QListWidget(this);
    groupLayout->addWidget(m_profileListWidget);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("添加截面", this);
    QPushButton* removeBtn = new QPushButton("移除选中", this);
    QPushButton* clearBtn = new QPushButton("全部清除", this);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(clearBtn);
    groupLayout->addLayout(buttonLayout);

    m_mainLayout->addWidget(selectionGroup);

    // OK and Cancel buttons
    QHBoxLayout* controlButtonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("确定", this);
    QPushButton* cancelButton = new QPushButton("取消", this);
    m_okButton->setEnabled(false); // Initially disabled

    controlButtonLayout->addStretch();
    controlButtonLayout->addWidget(cancelButton);
    controlButtonLayout->addWidget(m_okButton);
    m_mainLayout->addLayout(controlButtonLayout);

    // Connect signals and slots
    connect(addBtn, &QPushButton::clicked, this, &LoftFeatureDialog::onAddSectionClicked);
    connect(removeBtn, &QPushButton::clicked, this, &LoftFeatureDialog::onRemoveSectionClicked);
    connect(clearBtn, &QPushButton::clicked, this, &LoftFeatureDialog::onClearSectionsClicked);
    connect(m_okButton, &QPushButton::clicked, this, &LoftFeatureDialog::onOkButtonClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LoftFeatureDialog::onAddSectionClicked() {
    // 通知主窗口，我们需要用户去选择一个草图
    emit selectionModeChanged(true, "请在文档树中选择一个草图作为放样截面");
}

void LoftFeatureDialog::onSketchSelected(const cad_sketch::SketchPtr& sketch) {
    if (sketch) {
        // 将选中的草图添加到列表中
        m_selectedProfiles.push_back(sketch);
        updateProfileList();
    }
    // 退出选择模式
    emit selectionModeChanged(false, "");
}

void LoftFeatureDialog::onRemoveSectionClicked() {
    int currentRow = m_profileListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_selectedProfiles.size()) {
        m_selectedProfiles.erase(m_selectedProfiles.begin() + currentRow);
        updateProfileList();
    }
}

void LoftFeatureDialog::onClearSectionsClicked() {
    m_selectedProfiles.clear();
    updateProfileList();
}

void LoftFeatureDialog::onOkButtonClicked() {
    if (m_selectedProfiles.size() < 2) {
        QMessageBox::warning(this, "错误", "请至少选择两个截面轮廓来进行放样。");
        return;
    }
    emit operationRequested(m_selectedProfiles);
    accept();
}

void LoftFeatureDialog::updateProfileList() {
    m_profileListWidget->clear();
    for (const auto& profile : m_selectedProfiles) {
        m_profileListWidget->addItem(QString::fromStdString(profile->GetName()));
    }
    // 只有当截面数量大于等于2时，才允许点击“确定”
    m_okButton->setEnabled(m_selectedProfiles.size() >= 2);
}

} // namespace cad_ui