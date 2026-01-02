use anyhow::Result;
use chrono::{Local, Timelike};
use rodio::{source::SineWave, Decoder, OutputStream, Sink, Source};
use std::fs::File;
use std::io::BufReader;
use std::thread;
use std::time::Duration;

use crate::config::{ChimeMode, Config};

pub fn play_chime(config: &Config) -> Result<()> {
    let (_stream, stream_handle) = OutputStream::try_default()?;
    let sink = Sink::try_new(&stream_handle)?;

    match config.mode {
        ChimeMode::Notes => play_notes(&sink, &config.notes),
        ChimeMode::File => play_file(&sink, &config.file_path),
        ChimeMode::GrandfatherClock => play_grandfather_clock(&sink, config),
    }?;

    if config.mode != ChimeMode::GrandfatherClock {
        sink.sleep_until_end();
    }
    Ok(())
}

fn play_notes(sink: &Sink, notes: &str) -> Result<()> {
    // println!("Playing notes: {}", notes);
    
    let mut current_freq = 0.0;
    let mut current_duration_units = 0;
    let base_duration_ms = 300;

    for note in notes.split_whitespace() {
        if note == "-" {
            if current_duration_units > 0 {
                current_duration_units += 1;
            }
        } else {
            // Flush previous note or silence
            if current_duration_units > 0 {
                append_note(sink, current_freq, current_duration_units, base_duration_ms);
            }

            if note.eq_ignore_ascii_case("X") {
                current_freq = 0.0;
                current_duration_units = 1;
            } else {
                let freq = parse_note(note);
                if freq > 0.0 {
                    current_freq = freq;
                    current_duration_units = 1;
                } else {
                    // Reset if invalid note
                    current_freq = 0.0;
                    current_duration_units = 0;
                }
            }
        }
    }
    
    // Flush last note
    if current_duration_units > 0 {
        append_note(sink, current_freq, current_duration_units, base_duration_ms);
    }
    Ok(())
}

fn append_note(sink: &Sink, freq: f32, duration_units: u64, base_duration_ms: u64) {
    let duration = Duration::from_millis(base_duration_ms * duration_units);
    if freq > 0.0 {
        let source = SineWave::new(freq)
            .take_duration(duration)
            .amplify(0.20);
        sink.append(source);
    } else {
        // Silence (X) - Use a dummy frequency with 0 amplitude to avoid panic
        let source = SineWave::new(1.0)
            .take_duration(duration)
            .amplify(0.0);
        sink.append(source);
    }
}

fn play_file(sink: &Sink, file_path: &Option<String>) -> Result<()> {
    if let Some(path_str) = file_path {
        // println!("Playing file: {}", path_str);
        let file = File::open(path_str)?;
        let source = Decoder::new(BufReader::new(file))?;
        sink.append(source);
    } else {
        println!("No file path specified in config");
    }
    Ok(())
}

fn play_grandfather_clock(sink: &Sink, config: &Config) -> Result<()> {
    // 1. Play Prelude if it exists
    if let Some(prelude_path) = &config.prelude_file_path {
        // println!("Playing prelude: {}", prelude_path);
        let file = File::open(prelude_path)?;
        let source = Decoder::new(BufReader::new(file))?;
        sink.append(source);
        // Wait for prelude to finish before starting strikes
        sink.sleep_until_end(); 
    }

    // 2. Play Strikes
    if let Some(path_str) = &config.file_path {
        let now = Local::now();
        let (_is_pm, hour_12) = now.hour12();
        // For testing purposes, if hour is 0 (midnight/noon), treat as 12
        let count = if hour_12 == 0 { 12 } else { hour_12 };
        
        // println!("Playing strike: {} ({} times)", path_str, count);
        
        for i in 0..count {
            // We spawn a new thread for each strike to allow them to overlap naturally.
            let path_clone = path_str.clone();
            thread::spawn(move || {
                if let Ok((_stream, stream_handle)) = OutputStream::try_default()
                    && let Ok(sink) = Sink::try_new(&stream_handle)
                    && let Ok(file) = File::open(path_clone)
                    && let Ok(source) = Decoder::new(BufReader::new(file))
                {
                    sink.append(source);
                    sink.sleep_until_end();
                }
            });
            
            if i < count - 1 {
                thread::sleep(Duration::from_millis(config.strike_interval_ms));
            }
        }
        
        // Wait a bit to ensure the last strike has time to play before the main thread potentially exits
        // (relevant for CLI/Test mode)
        thread::sleep(Duration::from_secs(5));
        
    } else {
        println!("No strike file path specified in config");
    }
    Ok(())
}

fn parse_note(note: &str) -> f32 {
    // Simple parser: C4, A#4, etc. Default octave 4 if not specified.
    // Base frequencies for Octave 4: C4 = 261.63, A4 = 440.00
    
    let note = note.to_uppercase();
    let mut chars = note.chars();
    
    let base_note = match chars.next() {
        Some(c) => c,
        None => return 0.0,
    };

    let mut semitone_offset = match base_note {
        'C' => -9,
        'D' => -7,
        'E' => -5,
        'F' => -4,
        'G' => -2,
        'A' => 0,
        'B' => 2,
        _ => return 0.0,
    };

    // Check for sharp/flat
    let mut next_char = chars.next();
    if let Some(c) = next_char {
        if c == '#' {
            semitone_offset += 1;
            next_char = chars.next();
        } else if c == 'B' { // 'b' for flat
            semitone_offset -= 1;
            next_char = chars.next();
        }
    }

    // Check for octave
    let octave = if let Some(c) = next_char {
        c.to_digit(10).unwrap_or(4) as i32
    } else {
        4
    };

    let octave_diff = octave - 4;
    let total_semitones = semitone_offset + (octave_diff * 12);
    
    440.0 * 2.0_f32.powf(total_semitones as f32 / 12.0)
}
