#include "preferenceeditor.h"
#include "ui_preferenceeditor.h"
#include "config.h"
#include "noscrollcombobox.h"

#include <QAbstractButton>
#include <QRegularExpression>
#include <QDirIterator>
#include <QFormLayout>


PreferenceEditor::PreferenceEditor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PreferenceEditor),
    themeSelector(nullptr)
{
    ui->setupUi(this);
    auto *formLayout = new QFormLayout(ui->groupBox_Themes);
    themeSelector = new NoScrollComboBox(ui->groupBox_Themes);
    themeSelector->setEditable(false);
    themeSelector->setMinimumContentsLength(0);
    formLayout->addRow("Themes", themeSelector);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->buttonBox, &QDialogButtonBox::clicked,
            this, &PreferenceEditor::dialogButtonClicked);
    initFields();
    updateFields();
}

PreferenceEditor::~PreferenceEditor()
{
    delete ui;
}

void PreferenceEditor::initFields() {
    QStringList themes = { "default" };
    static const QRegularExpression re(":/themes/([A-z0-9_-]+).qss");
    QDirIterator it(":/themes", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString themeName = re.match(it.next()).captured(1);
        if (!themes.contains(themeName)) {
            themes.append(themeName);
        }
    }
    themeSelector->addItems(themes);
}

void PreferenceEditor::updateFields() {
    themeSelector->setTextItem(porymapConfig.theme);
    if (porymapConfig.eventSelectionShapeMode == QGraphicsPixmapItem::MaskShape) {
        ui->radioButton_OnSprite->setChecked(true);
    } else if (porymapConfig.eventSelectionShapeMode == QGraphicsPixmapItem::BoundingRectShape) {
        ui->radioButton_WithinRect->setChecked(true);
    }
    ui->lineEdit_TextEditorOpenFolder->setText(porymapConfig.textEditorOpenFolder);
    ui->lineEdit_TextEditorGotoLine->setText(porymapConfig.textEditorGotoLine);
    ui->checkBox_MonitorProjectFiles->setChecked(porymapConfig.monitorFiles);
    ui->checkBox_OpenRecentProject->setChecked(porymapConfig.reopenOnLaunch);
    ui->checkBox_CheckForUpdates->setChecked(porymapConfig.checkForUpdates);
    ui->checkBox_DisableEventWarning->setChecked(porymapConfig.eventDeleteWarningDisabled);
    ui->checkBox_AutocompleteAllScripts->setChecked(porymapConfig.loadAllEventScripts);

    auto logTypeEnd = porymapConfig.statusBarLogTypes.end();
    ui->checkBox_StatusErrors->setChecked(porymapConfig.statusBarLogTypes.find(LogType::LOG_ERROR) != logTypeEnd);
    ui->checkBox_StatusWarnings->setChecked(porymapConfig.statusBarLogTypes.find(LogType::LOG_WARN) != logTypeEnd);
    ui->checkBox_StatusInformation->setChecked(porymapConfig.statusBarLogTypes.find(LogType::LOG_INFO) != logTypeEnd);
}

void PreferenceEditor::saveFields() {
    if (themeSelector->currentText() != porymapConfig.theme) {
        const auto theme = themeSelector->currentText();
        porymapConfig.theme = theme;
        emit themeChanged(theme);
    }
    bool loadAllEventScripts = ui->checkBox_AutocompleteAllScripts->isChecked();
    if (loadAllEventScripts != porymapConfig.loadAllEventScripts) {
        porymapConfig.loadAllEventScripts = loadAllEventScripts;
        emit scriptSettingsChanged(loadAllEventScripts);
    }
    porymapConfig.eventSelectionShapeMode = ui->radioButton_OnSprite->isChecked() ? QGraphicsPixmapItem::MaskShape : QGraphicsPixmapItem::BoundingRectShape;
    porymapConfig.textEditorOpenFolder = ui->lineEdit_TextEditorOpenFolder->text();
    porymapConfig.textEditorGotoLine = ui->lineEdit_TextEditorGotoLine->text();
    porymapConfig.monitorFiles = ui->checkBox_MonitorProjectFiles->isChecked();
    porymapConfig.reopenOnLaunch = ui->checkBox_OpenRecentProject->isChecked();
    porymapConfig.checkForUpdates = ui->checkBox_CheckForUpdates->isChecked();
    porymapConfig.eventDeleteWarningDisabled = ui->checkBox_DisableEventWarning->isChecked();

    porymapConfig.statusBarLogTypes.clear();
    if (ui->checkBox_StatusErrors->isChecked()) porymapConfig.statusBarLogTypes.insert(LogType::LOG_ERROR);
    if (ui->checkBox_StatusWarnings->isChecked()) porymapConfig.statusBarLogTypes.insert(LogType::LOG_WARN);
    if (ui->checkBox_StatusInformation->isChecked()) porymapConfig.statusBarLogTypes.insert(LogType::LOG_INFO);

    porymapConfig.save();

    emit preferencesSaved();
}

void PreferenceEditor::dialogButtonClicked(QAbstractButton *button) {
    auto buttonRole = ui->buttonBox->buttonRole(button);
    if (buttonRole == QDialogButtonBox::AcceptRole) {
        saveFields();
        close();
    } else if (buttonRole == QDialogButtonBox::ApplyRole) {
        saveFields();
    } else if (buttonRole == QDialogButtonBox::RejectRole) {
        close();
    }
}
