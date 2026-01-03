#include "SynthGenerator.h"
#include <QtMath>
#include <QRegularExpression>

SynthGenerator::SynthGenerator(const QAudioFormat &format, QObject *parent)
    : QIODevice(parent)
    , m_format(format)
    , m_currentInstructionIndex(0)
    , m_samplesGeneratedInCurrentInstruction(0)
    , m_phase(0.0f)
    , m_volume(1.0f)
    , m_finished(true)
{
}

void SynthGenerator::start()
{
    open(QIODevice::ReadOnly);
    m_currentInstructionIndex = 0;
    m_samplesGeneratedInCurrentInstruction = 0;
    m_phase = 0.0f;
    m_finished = false;
}

void SynthGenerator::setSequence(const QString &notes, float speed, float volume)
{
    m_volume = volume;
    parseNotes(notes, speed);
}

qint64 SynthGenerator::readData(char *data, qint64 maxlen)
{
    if (m_finished) return 0;

    qint64 totalBytesWritten = 0;
    char *ptr = data;
    qint64 bytesRemaining = maxlen;

    while (bytesRemaining > 0 && m_currentInstructionIndex < m_instructions.size()) {
        NoteInstruction &instr = m_instructions[m_currentInstructionIndex];
        
        qint64 samplesRemainingInInstr = instr.durationSamples - m_samplesGeneratedInCurrentInstruction;
        qint64 bytesNeededForInstr = samplesRemainingInInstr * m_format.bytesPerFrame();
        
        qint64 bytesToWrite = qMin(bytesRemaining, bytesNeededForInstr);
        qint64 samplesToWrite = bytesToWrite / m_format.bytesPerFrame();

        float amplitude = 0.2f * m_volume;
        if (instr.frequency <= 0.0f) amplitude = 0.0f;

        // Assuming 16-bit signed integer format (Little Endian)
        qint16 *samplePtr = reinterpret_cast<qint16*>(ptr);
        
        if (instr.frequency > 0.0f) {
            float phaseStep = (instr.frequency * 2.0f * M_PI) / m_format.sampleRate();
            
            for (int i = 0; i < samplesToWrite; ++i) {
                float sampleVal = qSin(m_phase) * amplitude;
                qint16 pcmVal = static_cast<qint16>(sampleVal * 32767.0f);
                *samplePtr++ = pcmVal;
                if (m_format.channelCount() > 1) *samplePtr++ = pcmVal;
                
                m_phase += phaseStep;
                if (m_phase > 2.0f * M_PI) m_phase -= 2.0f * M_PI;
            }
        } else {
            memset(ptr, 0, bytesToWrite);
        }

        ptr += bytesToWrite;
        totalBytesWritten += bytesToWrite;
        bytesRemaining -= bytesToWrite;
        m_samplesGeneratedInCurrentInstruction += samplesToWrite;

        if (m_samplesGeneratedInCurrentInstruction >= instr.durationSamples) {
            m_currentInstructionIndex++;
            m_samplesGeneratedInCurrentInstruction = 0;
            m_phase = 0.0f;
        }
    }

    if (m_currentInstructionIndex >= m_instructions.size()) {
        m_finished = true;
    }

    return totalBytesWritten;
}

qint64 SynthGenerator::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return 0;
}

qint64 SynthGenerator::bytesAvailable() const
{
    if (m_finished) return 0;
    // Return a large number to ensure QAudioSink keeps reading until we return 0 in readData
    return m_instructions.size() > 0 ? 1024 * 1024 : 0; 
}

void SynthGenerator::parseNotes(const QString &notesStr, float speed)
{
    m_instructions.clear();
    
    QStringList tokens = notesStr.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    float currentFreq = 0.0f;
    int currentDurationUnits = 0;
    float baseDurationMs = 300.0f / speed;
    qint64 baseDurationSamples = static_cast<qint64>((baseDurationMs / 1000.0f) * m_format.sampleRate());

    for (const QString &token : tokens) {
        if (token == "-") {
            if (currentDurationUnits > 0) {
                currentDurationUnits++;
            }
        } else {
            if (currentDurationUnits > 0) {
                NoteInstruction instr;
                instr.frequency = currentFreq;
                instr.durationSamples = baseDurationSamples * currentDurationUnits;
                m_instructions.append(instr);
            }

            if (token.compare("X", Qt::CaseInsensitive) == 0 || token.compare("Z", Qt::CaseInsensitive) == 0) {
                currentFreq = 0.0f;
                currentDurationUnits = 1;
            } else if (token == "?") {
                // Random note C3 (-21) to C6 (+15)
                int semitoneOffset = QRandomGenerator::global()->bounded(-21, 16);
                currentFreq = 440.0f * qPow(2.0f, semitoneOffset / 12.0f);
                currentDurationUnits = 1;
            } else {
                float freq = parseNoteFreq(token);
                if (freq > 0.0f) {
                    currentFreq = freq;
                    currentDurationUnits = 1;
                } else {
                    currentFreq = 0.0f;
                    currentDurationUnits = 0;
                }
            }
        }
    }

    if (currentDurationUnits > 0) {
        NoteInstruction instr;
        instr.frequency = currentFreq;
        instr.durationSamples = baseDurationSamples * currentDurationUnits;
        m_instructions.append(instr);
    }
}

float SynthGenerator::parseNoteFreq(const QString &note)
{
    if (note.isEmpty()) return 0.0f;

    QString n = note.toUpper();
    int idx = 0;
    QChar baseChar = n[idx++];
    
    int semitoneOffset = 0;
    switch (baseChar.toLatin1()) {
        case 'C': semitoneOffset = -9; break;
        case 'D': semitoneOffset = -7; break;
        case 'E': semitoneOffset = -5; break;
        case 'F': semitoneOffset = -4; break;
        case 'G': semitoneOffset = -2; break;
        case 'A': semitoneOffset = 0; break;
        case 'B': semitoneOffset = 2; break;
        default: return 0.0f;
    }

    if (idx < n.length()) {
        QChar next = n[idx];
        if (next == '#') {
            semitoneOffset++;
            idx++;
        } else if (next == 'B') {
            semitoneOffset--;
            idx++;
        }
    }

    int octave = 4;
    if (idx < n.length()) {
        bool ok;
        int val = n.mid(idx).toInt(&ok);
        if (ok) octave = val;
    }

    int octaveDiff = octave - 4;
    int totalSemitones = semitoneOffset + (octaveDiff * 12);

    return 440.0f * qPow(2.0f, totalSemitones / 12.0f);
}
