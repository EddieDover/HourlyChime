use eframe::egui;
use rfd::AsyncFileDialog;
use std::sync::mpsc::{channel, Receiver, Sender};
use crate::config::{self, ChimeMode, Config};

pub fn run() -> eframe::Result<()> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([400.0, 400.0])
            .with_resizable(true),
        ..Default::default()
    };

    eframe::run_native(
        "Hourly Chime Settings",
        options,
        Box::new(|_cc| Ok(Box::new(SettingsApp::new()))),
    )
}

enum FileType {
    Prelude,
    Audio,
    Strike,
}

struct SettingsApp {
    config: Config,
    status_msg: Option<String>,
    rx: Receiver<(FileType, String)>,
    tx: Sender<(FileType, String)>,
}

impl SettingsApp {
    fn new() -> Self {
        let config = config::load_config().unwrap_or_default();
        let (tx, rx) = channel();
        Self {
            config,
            status_msg: None,
            rx,
            tx,
        }
    }
}

impl eframe::App for SettingsApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Check for file dialog results
        while let Ok((ftype, path)) = self.rx.try_recv() {
            match ftype {
                FileType::Prelude => self.config.prelude_file_path = Some(path),
                FileType::Audio => self.config.audio_file_path = Some(path),
                FileType::Strike => self.config.strike_file_path = Some(path),
            }
        }

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Hourly Chime Settings");
            ui.add_space(10.0);

            ui.label("Chime Mode:");
            ui.horizontal(|ui| {
                ui.radio_value(&mut self.config.mode, ChimeMode::Notes, "Notes");
                ui.radio_value(&mut self.config.mode, ChimeMode::File, "Audio File");
                ui.radio_value(&mut self.config.mode, ChimeMode::GrandfatherClock, "Grandfather Clock");
            });

            ui.add_space(10.0);

            match self.config.mode {
                ChimeMode::Notes => {
                    ui.label("Notes (e.g., 'C E G C5'):");
                    ui.text_edit_singleline(&mut self.config.notes);
                    ui.small("Supported: A-G, #/b, Octave (e.g. C#5)");
                    ui.small("Use '-' to hold previous note, 'X' for silence, '?' for random.");
                    
                    ui.add_space(5.0);
                    ui.label("Playback Speed:");
                    ui.add(egui::Slider::new(&mut self.config.note_speed, 0.1..=5.0).text("x"));

                    ui.add_space(10.0);
                    if ui.button("Reset Defaults").clicked() {
                        self.config.notes = "C E G C5".to_string();
                        self.config.note_speed = 1.0;
                    }
                }
                ChimeMode::File => {
                    ui.label("Audio File Path:");
                    ui.horizontal(|ui| {
                        let mut path_display = self.config.audio_file_path.clone().unwrap_or_default();
                        ui.text_edit_singleline(&mut path_display); // Read-only-ish display
                        
                        if ui.button("Browse...").clicked() {
                            let tx = self.tx.clone();
                            tokio::spawn(async move {
                                if let Some(file) = AsyncFileDialog::new()
                                    .add_filter("Audio", &["mp3", "wav", "ogg"])
                                    .pick_file()
                                    .await 
                                {
                                    let _ = tx.send((FileType::Audio, file.path().to_string_lossy().to_string()));
                                }
                            });
                        }
                    });
                    if let Some(path) = &self.config.audio_file_path {
                        ui.label(format!("Selected: {}", path));
                    }

                    ui.add_space(10.0);
                    if ui.button("Reset Defaults").clicked() {
                        if let Ok((chime_path, _)) = config::ensure_assets() {
                            self.config.audio_file_path = Some(chime_path.to_string_lossy().to_string());
                        }
                    }
                }
                ChimeMode::GrandfatherClock => {
                    ui.label("Grandfather Clock Mode:");
                    ui.small("Plays the prelude (optional), then the strike file X times.");
                    
                    ui.add_space(5.0);
                    ui.label("Prelude File (Optional):");
                    ui.horizontal(|ui| {
                        let mut path_display = self.config.prelude_file_path.clone().unwrap_or_default();
                        ui.text_edit_singleline(&mut path_display);
                        
                        if ui.button("Browse...").clicked() {
                            let tx = self.tx.clone();
                            tokio::spawn(async move {
                                if let Some(file) = AsyncFileDialog::new()
                                    .add_filter("Audio", &["mp3", "wav", "ogg"])
                                    .pick_file()
                                    .await 
                                {
                                    let _ = tx.send((FileType::Prelude, file.path().to_string_lossy().to_string()));
                                }
                            });
                        }
                    });
                    
                    ui.add_space(5.0);
                    ui.label("Strike Interval (ms):");
                    ui.add(egui::DragValue::new(&mut self.config.strike_interval_ms).range(100..=10000));
                    ui.add_space(5.0);
                    
                    ui.label("Strike File Path:");
                    ui.horizontal(|ui| {
                        let mut path_display = self.config.strike_file_path.clone().unwrap_or_default();
                        ui.text_edit_singleline(&mut path_display); // Read-only-ish display
                        
                        if ui.button("Browse...").clicked() {
                            let tx = self.tx.clone();
                            tokio::spawn(async move {
                                if let Some(file) = AsyncFileDialog::new()
                                    .add_filter("Audio", &["mp3", "wav", "ogg"])
                                    .pick_file()
                                    .await 
                                {
                                    let _ = tx.send((FileType::Strike, file.path().to_string_lossy().to_string()));
                                }
                            });
                        }
                    });
                    if let Some(path) = &self.config.strike_file_path {
                        ui.label(format!("Selected: {}", path));
                    }

                    ui.add_space(10.0);
                    if ui.button("Reset Defaults").clicked() {
                        if let Ok((chime_path, prelude_path)) = config::ensure_assets() {
                            self.config.prelude_file_path = Some(prelude_path.to_string_lossy().to_string());
                            self.config.strike_file_path = Some(chime_path.to_string_lossy().to_string());
                        }
                        self.config.strike_interval_ms = 2000;
                    }
                }
            }

            ui.add_space(20.0);

            if ui.button("Test Sound").clicked() {
                let config_clone = self.config.clone();
                std::thread::spawn(move || {
                    if let Err(e) = crate::audio::play_chime(&config_clone) {
                        eprintln!("Error playing test sound: {}", e);
                    }
                });
            }

            ui.add_space(10.0);

            if ui.button("Save Settings").clicked() {
                match config::save_config(&self.config) {
                    Ok(_) => self.status_msg = Some("Settings saved successfully!".to_string()),
                    Err(e) => self.status_msg = Some(format!("Error saving: {}", e)),
                }
            }

            if let Some(msg) = &self.status_msg {
                ui.add_space(10.0);
                ui.label(msg);
            }
        });
    }
}
