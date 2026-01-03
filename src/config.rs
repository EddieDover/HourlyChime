use anyhow::Result;
use directories::ProjectDirs;
use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
pub enum ChimeMode {
    Notes,
    File,
    GrandfatherClock,
}

#[derive(Debug, Serialize, Deserialize, Clone, PartialEq)]
pub struct Config {
    pub mode: ChimeMode,
    pub notes: String,
    pub note_speed: f32,
    pub audio_file_path: Option<String>,
    pub strike_file_path: Option<String>,
    pub prelude_file_path: Option<String>,
    pub strike_interval_ms: u64,
    #[serde(default = "default_volume")]
    pub volume: f32,
}

fn default_volume() -> f32 {
    1.0
}

impl Default for Config {
    fn default() -> Self {
        Self {
            mode: ChimeMode::Notes,
            notes: "C E G C5".to_string(),
            note_speed: 1.0,
            audio_file_path: None,
            strike_file_path: None,
            prelude_file_path: None,
            strike_interval_ms: 2000,
            volume: 1.0,
        }
    }
}

pub fn get_config_dir() -> Result<PathBuf> {
    if let Some(proj_dirs) = ProjectDirs::from("com", "HourlyChime", "HourlyChime") {
        let config_dir = proj_dirs.config_dir();
        if !config_dir.exists() {
            fs::create_dir_all(config_dir)?;
        }
        Ok(config_dir.to_path_buf())
    } else {
        anyhow::bail!("Could not determine config directory")
    }
}

pub fn get_config_path() -> Result<PathBuf> {
    Ok(get_config_dir()?.join("config.json"))
}

const CHIME_MP3: &[u8] = include_bytes!("../assets/sounds/gc-chime.mp3");
const PRELUDE_MP3: &[u8] = include_bytes!("../assets/sounds/gc-prelude.mp3");

pub fn ensure_assets() -> Result<(PathBuf, PathBuf)> {
    let config_dir = get_config_dir()?;
    let sounds_dir = config_dir.join("sounds");
    if !sounds_dir.exists() {
        fs::create_dir_all(&sounds_dir)?;
    }

    let chime_path = sounds_dir.join("gc-chime.mp3");
    if !chime_path.exists() {
        fs::write(&chime_path, CHIME_MP3)?;
    }

    let prelude_path = sounds_dir.join("gc-prelude.mp3");
    if !prelude_path.exists() {
        fs::write(&prelude_path, PRELUDE_MP3)?;
    }

    Ok((chime_path, prelude_path))
}

pub fn load_config() -> Result<Config> {
    let config_path = get_config_path()?;
    if config_path.exists() {
        let content = fs::read_to_string(config_path)?;
        let config: Config = serde_json::from_str(&content)?;
        Ok(config)
    } else {
        let config = Config::default();
        save_config(&config)?;
        Ok(config)
    }
}

pub fn save_config(config: &Config) -> Result<()> {
    let config_path = get_config_path()?;
    let content = serde_json::to_string_pretty(config)?;
    fs::write(config_path, content)?;
    Ok(())
}
