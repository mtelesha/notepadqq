#include "include/frmpreferences.h"
#include "include/editor.h"
#include "ui_frmpreferences.h"

frmPreferences::frmPreferences(TopEditorContainer *topEditorContainer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmPreferences),
    m_topEditorContainer(topEditorContainer),
    m_langsTempSettings(new QMap<QString, QVariant>())
{
    ui->setupUi(this);

    // Select first item in treeWidget
    ui->treeWidget->setCurrentItem(ui->treeWidget->topLevelItem(0));

    QSettings s;

    ui->chkCheckQtVersionAtStartup->setChecked(s.value("checkQtVersionAtStartup", true).toBool());

    loadLanguages(&s);
}

frmPreferences::~frmPreferences()
{
    delete ui;
    delete m_langsTempSettings;
    delete m_commonLanguageProperties;
}

void frmPreferences::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem * /*previous*/)
{
    int index = ui->treeWidget->indexOfTopLevelItem(current);

    if (index != -1) {
        ui->stackedWidget->setCurrentIndex(index);
    }
}

void frmPreferences::on_buttonBox_accepted()
{
    QSettings s;
    s.setValue("checkQtVersionAtStartup", ui->chkCheckQtVersionAtStartup->isChecked());

    saveLanguages(&s);

    accept();
}

void frmPreferences::loadLanguages(QSettings *s)
{
    m_commonLanguageProperties = new QVariantMap();
    m_commonLanguageProperties->insert("tabSize", 4);
    m_commonLanguageProperties->insert("indentWithSpaces", false);


    m_langs = m_topEditorContainer->currentTabWidget()->currentEditor()->languages();

    std::sort(m_langs.begin(), m_langs.end(), Editor::LanguageGreater());

    ui->cmbLanguages->addItem("Default", QVariant(QString()));
    QString keyPrefix = "Languages/";

    for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
        m_langsTempSettings->insert(keyPrefix + prop.key(), s->value(keyPrefix + prop.key(), prop.value()));
    }

    for (int i = 0; i < m_langs.length(); i++) {
        const QMap<QString, QString> &map = m_langs.at(i);

        QString langId = map.value("id", "");
        ui->cmbLanguages->addItem(map.value("name", "?"), langId);

        QString keyPrefix = "Languages/" + langId + "/";

        m_langsTempSettings->insert(keyPrefix + "useDefaultSettings", s->value(keyPrefix + "useDefaultSettings", true));

        for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
            m_langsTempSettings->insert(keyPrefix + prop.key(), s->value(keyPrefix + prop.key(), prop.value()));
        }
    }

    ui->cmbLanguages->setCurrentIndex(0);
    ui->cmbLanguages->currentIndexChanged(0);
}

void frmPreferences::saveLanguages(QSettings *s)
{
    QString keyPrefix = "Languages/";

    for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
        s->setValue(keyPrefix + prop.key(), m_langsTempSettings->value(keyPrefix + prop.key()));
    }

    for (int i = 0; i < m_langs.length(); i++) {
        const QMap<QString, QString> &map = m_langs.at(i);

        QString langId = map.value("id", "");
        QString keyPrefix = "Languages/" + langId + "/";

        s->setValue(keyPrefix + "useDefaultSettings", m_langsTempSettings->value(keyPrefix + "useDefaultSettings"));

        for (QVariantMap::iterator prop = m_commonLanguageProperties->begin(); prop != m_commonLanguageProperties->end(); ++prop) {
            s->setValue(keyPrefix + prop.key(), m_langsTempSettings->value(keyPrefix + prop.key()));
        }
    }
}

void frmPreferences::on_buttonBox_rejected()
{
    reject();
}

void frmPreferences::on_cmbLanguages_currentIndexChanged(int index)
{
    QVariant data = ui->cmbLanguages->itemData(index);

    QString keyPrefix;

    if (data.isNull()) {
        // General

        // Hide "use default settings" checkbox, and enable the other stuff
        ui->chkLanguages_useDefaultSettings->setVisible(false);
        ui->frameLanguages->setEnabled(true);

        keyPrefix = "Languages/";
    } else {
        QString langId = data.toString();
        keyPrefix = "Languages/" + langId + "/";

        // Show "use default settings" checkbox
        ui->chkLanguages_useDefaultSettings->setVisible(true);

        // Load "use default settings" value
        bool usingDefault = m_langsTempSettings->value(keyPrefix + "useDefaultSettings").toBool();
        ui->chkLanguages_useDefaultSettings->setChecked(usingDefault);
        ui->chkLanguages_useDefaultSettings->toggled(usingDefault);
    }

    ui->txtLanguages_TabSize->setValue(m_langsTempSettings->value(keyPrefix + "tabSize").toInt());
    ui->chkLanguages_IndentWithSpaces->setChecked(m_langsTempSettings->value(keyPrefix + "indentWithSpaces").toBool());
}

void frmPreferences::on_chkLanguages_useDefaultSettings_toggled(bool checked)
{
    ui->frameLanguages->setEnabled(!checked);

    QVariant langId = ui->cmbLanguages->currentData();
    if (langId.isNull() == false) {
        QString keyPrefix = "Languages/" + langId.toString() + "/";
        m_langsTempSettings->insert(keyPrefix + "useDefaultSettings", checked);
    }
}

void frmPreferences::setCurrentLanguageTempValue(QString key, QVariant value)
{
    QVariant langId = ui->cmbLanguages->currentData();

    QString keyPrefix;
    if (langId.isNull())
        keyPrefix = "Languages/";
    else
        keyPrefix = "Languages/" + langId.toString() + "/";

    m_langsTempSettings->insert(keyPrefix + key, value);
}

void frmPreferences::on_txtLanguages_TabSize_valueChanged(int value)
{
    setCurrentLanguageTempValue("tabSize", value);
}

void frmPreferences::on_chkLanguages_IndentWithSpaces_toggled(bool checked)
{
    setCurrentLanguageTempValue("indentWithSpaces", checked);
}
