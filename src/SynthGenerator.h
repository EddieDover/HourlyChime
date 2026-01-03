#ifndef SYNTHGENERATOR_H
#define SYNTHGENERATOR_H

#include <QIODevice>
#include <QAudioFormat>
#include <QVector>
#include <QRandomGenerator>

struct NoteInstruction {
    float frequency; // 0.0 for silence
    qint64 durationSamples;
};

class SynthGenerator : public QIODevice
{
    Q_OBJECT

public:
    explicit SynthGenerator(const QAudioFormat &format, QObject *parent = nullptr);
    
    void setSequence(const QString &notes, float speed, float volume);
    void start();

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    qint64 bytesAvailable() const override;

private:
    void parseNotes(const QString &notes, float speed);
    float parseNoteFreq(const QString &note);
    void generateSine(char *data, qint64 maxlen);

    QAudioFormat m_format;
    QVector<NoteInstruction> m_instructions;
    int m_currentInstructionIndex;
    qint64 m_samplesGeneratedInCurrentInstruction;
    float m_phase;
    float m_volume;
    bool m_finished;
};

#endif // SYNTHGENERATOR_H
