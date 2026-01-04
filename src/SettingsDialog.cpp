#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QFileDialog>
#include <QGroupBox>
#include <QFormLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Hourly Chime Settings"));
    resize(400, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *modeGroup = new QGroupBox(tr("Chime Mode"), this);
    QVBoxLayout *modeLayout = new QVBoxLayout(modeGroup);
    modeCombo = new QComboBox(this);
    modeCombo->addItem("Notes", "Notes");
    modeCombo->addItem("Single File", "File");
    modeCombo->addItem("Grandfather Clock", "GrandfatherClock");
    modeLayout->addWidget(modeCombo);
    mainLayout->addWidget(modeGroup);

    QGroupBox *notesGroup = new QGroupBox(tr("Notes Configuration"), this);
    QFormLayout *notesLayout = new QFormLayout(notesGroup);
    notesEdit = new QLineEdit(this);
    noteSpeedSpin = new QDoubleSpinBox(this);
    noteSpeedSpin->setRange(0.1, 5.0);
    noteSpeedSpin->setSingleStep(0.1);
    notesLayout->addRow(tr("Notes:"), notesEdit);
    notesLayout->addRow(tr("Speed:"), noteSpeedSpin);
    mainLayout->addWidget(notesGroup);

    QGroupBox *fileGroup = new QGroupBox(tr("File Configuration"), this);
    QGridLayout *fileLayout = new QGridLayout(fileGroup);
    
    audioFileEdit = new QLineEdit(this);
    browseAudioBtn = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(new QLabel(tr("Audio File:")), 0, 0);
    fileLayout->addWidget(audioFileEdit, 0, 1);
    fileLayout->addWidget(browseAudioBtn, 0, 2);

    strikeFileEdit = new QLineEdit(this);
    browseStrikeBtn = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(new QLabel(tr("Strike File:")), 1, 0);
    fileLayout->addWidget(strikeFileEdit, 1, 1);
    fileLayout->addWidget(browseStrikeBtn, 1, 2);

    preludeFileEdit = new QLineEdit(this);
    browsePreludeBtn = new QPushButton(tr("Browse..."), this);
    fileLayout->addWidget(new QLabel(tr("Prelude File:")), 2, 0);
    fileLayout->addWidget(preludeFileEdit, 2, 1);
    fileLayout->addWidget(browsePreludeBtn, 2, 2);

    strikeIntervalSpin = new QSpinBox(this);
    strikeIntervalSpin->setRange(-1, 10000);
    strikeIntervalSpin->setSpecialValueText(tr("Disabled"));
    strikeIntervalSpin->setSuffix(" ms");
    fileLayout->addWidget(new QLabel(tr("Strike Interval:")), 3, 0);
    fileLayout->addWidget(strikeIntervalSpin, 3, 1);

    mainLayout->addWidget(fileGroup);

    QGroupBox *generalGroup = new QGroupBox(tr("General"), this);
    QFormLayout *generalLayout = new QFormLayout(generalGroup);
    volumeSpin = new QDoubleSpinBox(this);
    volumeSpin->setRange(0.0, 1.0);
    volumeSpin->setSingleStep(0.1);
    generalLayout->addRow(tr("Volume:"), volumeSpin);
    mainLayout->addWidget(generalGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    testBtn = new QPushButton(tr("Test Sound"), this);
    QPushButton *resetBtn = new QPushButton(tr("Reset Defaults"), this);
    QPushButton *saveBtn = new QPushButton(tr("Save"), this);
    QPushButton *cancelBtn = new QPushButton(tr("Close"), this);
    btnLayout->addWidget(testBtn);
    btnLayout->addWidget(resetBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(testBtn, &QPushButton::clicked, this, &SettingsDialog::testSettings);
    connect(resetBtn, &QPushButton::clicked, this, &SettingsDialog::resetDefaults);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::reject);
    connect(browseAudioBtn, &QPushButton::clicked, this, &SettingsDialog::browseAudioFile);
    connect(browseStrikeBtn, &QPushButton::clicked, this, &SettingsDialog::browseStrikeFile);
    connect(browsePreludeBtn, &QPushButton::clicked, this, &SettingsDialog::browsePreludeFile);
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::updateUiState);

    Config::AppConfig cfg = Config::load();
    int index = modeCombo->findData(cfg.mode);
    if (index != -1) modeCombo->setCurrentIndex(index);

    notesEdit->setText(cfg.notes);
    noteSpeedSpin->setValue(cfg.noteSpeed);
    audioFileEdit->setText(cfg.audioFilePath);
    strikeFileEdit->setText(cfg.strikeFilePath);
    preludeFileEdit->setText(cfg.preludeFilePath);
    strikeIntervalSpin->setValue(cfg.strikeIntervalMs);
    volumeSpin->setValue(cfg.volume);

    updateUiState();
}

void SettingsDialog::updateUiState()
{
    QString mode = modeCombo->currentData().toString();
    bool isNotes = (mode == "Notes");
    bool isFile = (mode == "File");
    bool isGrandfather = (mode == "GrandfatherClock");

    notesEdit->setEnabled(isNotes);
    noteSpeedSpin->setEnabled(isNotes);
    
    audioFileEdit->setEnabled(isFile);
    browseAudioBtn->setEnabled(isFile);

    strikeFileEdit->setEnabled(isGrandfather);
    browseStrikeBtn->setEnabled(isGrandfather);
    preludeFileEdit->setEnabled(isGrandfather);
    browsePreludeBtn->setEnabled(isGrandfather);
    strikeIntervalSpin->setEnabled(isGrandfather);
}

void SettingsDialog::saveSettings()
{
    Config::AppConfig cfg;
    cfg.mode = modeCombo->currentData().toString();
    cfg.notes = notesEdit->text();
    cfg.noteSpeed = noteSpeedSpin->value();
    cfg.audioFilePath = audioFileEdit->text();
    cfg.strikeFilePath = strikeFileEdit->text();
    cfg.preludeFilePath = preludeFileEdit->text();
    cfg.strikeIntervalMs = strikeIntervalSpin->value();
    cfg.volume = volumeSpin->value();
    
    Config::save(cfg);
    
    emit configChanged();
}

void SettingsDialog::resetDefaults()
{
    Config::AppConfig cfg = Config::getDefaults();
    
    int index = modeCombo->findData(cfg.mode);
    if (index != -1) modeCombo->setCurrentIndex(index);

    notesEdit->setText(cfg.notes);
    noteSpeedSpin->setValue(cfg.noteSpeed);
    audioFileEdit->setText(cfg.audioFilePath);
    strikeFileEdit->setText(cfg.strikeFilePath);
    preludeFileEdit->setText(cfg.preludeFilePath);
    strikeIntervalSpin->setValue(cfg.strikeIntervalMs);
    volumeSpin->setValue(cfg.volume);

    updateUiState();
}

void SettingsDialog::testSettings()
{
    if (testBtn->text() == tr("Stop Test")) {
        emit stopTestRequested();
        
        testBtn->setText(tr("Test Sound"));
        return;
    }

    Config::AppConfig cfg;
    cfg.mode = modeCombo->currentData().toString();
    cfg.notes = notesEdit->text();
    cfg.noteSpeed = noteSpeedSpin->value();
    cfg.audioFilePath = audioFileEdit->text();
    cfg.strikeFilePath = strikeFileEdit->text();
    cfg.preludeFilePath = preludeFileEdit->text();
    cfg.strikeIntervalMs = strikeIntervalSpin->value();
    cfg.volume = volumeSpin->value();

    if (cfg.mode == "File" || cfg.mode == "GrandfatherClock") {
        testBtn->setText(tr("Stop Test"));
    }

    emit testRequested(cfg);
}

void SettingsDialog::onTestFinished()
{
    testBtn->setText(tr("Test Sound"));
}

void SettingsDialog::browseAudioFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select Audio File"), "", tr("Audio Files (*.mp3 *.wav *.ogg)"));
    if (!path.isEmpty()) audioFileEdit->setText(path);
}

void SettingsDialog::browseStrikeFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select Strike File"), "", tr("Audio Files (*.mp3 *.wav *.ogg)"));
    if (!path.isEmpty()) strikeFileEdit->setText(path);
}

void SettingsDialog::browsePreludeFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Select Prelude File"), "", tr("Audio Files (*.mp3 *.wav *.ogg)"));
    if (!path.isEmpty()) preludeFileEdit->setText(path);
}
