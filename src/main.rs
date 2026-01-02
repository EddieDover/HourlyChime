use anyhow::Result;
use chrono::{Local, Timelike};
use std::process::Command;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;
use tao::event_loop::{ControlFlow, EventLoopBuilder};

mod audio;
mod config;
mod gui;
mod tray;

const ICON_RGBA: &[u8] = include_bytes!("../assets/images/icon.rgba");

fn main() -> Result<()> {
    // Initialize Tokio runtime
    let rt = tokio::runtime::Runtime::new()?;
    let _guard = rt.enter();

    // Check for command line arguments to see if we should run GUI
    let args: Vec<String> = std::env::args().collect();
    if args.len() > 1 && args[1] == "--settings" {
        if let Err(e) = gui::run() {
            eprintln!("GUI Error: {}", e);
        }
        return Ok(());
    }

    let event_loop = EventLoopBuilder::<tray::TrayEvent>::with_user_event().build();
    let proxy = event_loop.create_proxy();

    // Ensure assets exist and update config if needed
    let (chime_path, prelude_path) = config::ensure_assets()?;

    // Update config defaults if they are missing
    if let Ok(mut cfg) = config::load_config() {
        let mut changed = false;
        if cfg.audio_file_path.is_none() {
            cfg.audio_file_path = Some(chime_path.to_string_lossy().to_string());
            changed = true;
        }
        if cfg.strike_file_path.is_none() {
            cfg.strike_file_path = Some(chime_path.to_string_lossy().to_string());
            changed = true;
        }
        if cfg.prelude_file_path.is_none() {
            cfg.prelude_file_path = Some(prelude_path.to_string_lossy().to_string());
            changed = true;
        }
        
        if changed {
            let _ = config::save_config(&cfg);
        }
    }

    // Setup Tray
    let _tray = tray::setup_tray(Arc::new(move |event| {
        let _ = proxy.send_event(event);
    }), ICON_RGBA)?;

    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    // Background thread for time checking
    thread::spawn(move || {
        let mut last_hour = Local::now().hour();
        
        while r.load(Ordering::Relaxed) {
            let now = Local::now();
            let current_hour = now.hour();

            // Check if we crossed an hour boundary
            if current_hour != last_hour {
                println!("It's the hour! Playing chime...");
                if let Ok(cfg) = config::load_config() {
                    if let Err(e) = audio::play_chime(&cfg) {
                        eprintln!("Error playing chime: {}", e);
                    }
                } else {
                    eprintln!("Could not load config");
                }
                last_hour = current_hour;
            }

            thread::sleep(Duration::from_secs(1));
        }
    });

    // Event Loop
    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;

        if let tao::event::Event::UserEvent(tray_event) = event {
            match tray_event {
                tray::TrayEvent::Quit => {
                    running.store(false, Ordering::Relaxed);
                    *control_flow = ControlFlow::Exit;
                }
                tray::TrayEvent::Settings => {
                    // Spawn a new instance of ourselves with --settings argument
                    if let Ok(exe_path) = std::env::current_exe() {
                        match Command::new(exe_path).arg("--settings").spawn() {
                            Ok(_) => {},
                            Err(e) => eprintln!("Failed to spawn settings: {}", e),
                        }
                    }
                }
            }
        }
    });
}
