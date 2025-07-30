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
    setWindowTitle("������������");
    resize(400, 350);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(10);

    QGroupBox* selectionGroup = new QGroupBox("�������� (Profiles)", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(selectionGroup);

    m_profileListWidget = new QListWidget(this);
    groupLayout->addWidget(m_profileListWidget);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("��ӽ���", this);
    QPushButton* removeBtn = new QPushButton("�Ƴ�ѡ��", this);
    QPushButton* clearBtn = new QPushButton("ȫ�����", this);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addWidget(clearBtn);
    groupLayout->addLayout(buttonLayout);

    m_mainLayout->addWidget(selectionGroup);

    // OK and Cancel buttons
    QHBoxLayout* controlButtonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("ȷ��", this);
    QPushButton* cancelButton = new QPushButton("ȡ��", this);
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
    // ֪ͨ�����ڣ�������Ҫ�û�ȥѡ��һ����ͼ
    emit selectionModeChanged(true, "�����ĵ�����ѡ��һ����ͼ��Ϊ��������");
}

void LoftFeatureDialog::onSketchSelected(const cad_sketch::SketchPtr& sketch) {
    if (sketch) {
        // ��ѡ�еĲ�ͼ��ӵ��б���
        m_selectedProfiles.push_back(sketch);
        updateProfileList();
    }
    // �˳�ѡ��ģʽ
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
        QMessageBox::warning(this, "����", "������ѡ�������������������з�����");
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
    // ֻ�е������������ڵ���2ʱ������������ȷ����
    m_okButton->setEnabled(m_selectedProfiles.size() >= 2);
}

} // namespace cad_ui