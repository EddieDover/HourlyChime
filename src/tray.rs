use std::sync::Arc;

pub enum TrayEvent {
    Settings,
    Quit,
}

pub type TrayCallback = Arc<dyn Fn(TrayEvent) + Send + Sync + 'static>;

#[cfg(target_os = "linux")]
pub use linux::setup_tray;

#[cfg(not(target_os = "linux"))]
pub use other::setup_tray;

#[cfg(target_os = "linux")]
mod linux {
    use super::{TrayEvent, TrayCallback};
    use ksni::{Tray, MenuItem, menu::StandardItem};
    use ksni::blocking::TrayMethods;

    struct HourlyTray {
        callback: TrayCallback,
        icon_data: Vec<u8>,
    }

    impl std::fmt::Debug for HourlyTray {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            write!(f, "HourlyTray")
        }
    }

    impl Tray for HourlyTray {
        fn id(&self) -> String { "hourly-chime".into() }
        fn icon_name(&self) -> String { "".into() } // Empty to force pixmap usage
        fn title(&self) -> String { "Hourly Chime".into() }
        
        fn icon_pixmap(&self) -> Vec<ksni::Icon> {
            // Convert RGBA to ARGB
            let argb_data: Vec<u8> = self.icon_data
                .chunks_exact(4)
                .flat_map(|chunk| {
                    let r = chunk[0];
                    let g = chunk[1];
                    let b = chunk[2];
                    let a = chunk[3];
                    vec![a, r, g, b]
                })
                .collect();

            vec![ksni::Icon {
                width: 32,
                height: 32,
                data: argb_data,
            }]
        }

        fn menu(&self) -> Vec<MenuItem<Self>> {
            let cb_settings = self.callback.clone();
            let cb_quit = self.callback.clone();

            vec![
                StandardItem {
                    label: "Settings".into(),
                    activate: Box::new(move |_| {
                        cb_settings(TrayEvent::Settings);
                    }),
                    ..Default::default()
                }.into(),
                StandardItem {
                    label: "Quit".into(),
                    activate: Box::new(move |_| {
                        cb_quit(TrayEvent::Quit);
                    }),
                    ..Default::default()
                }.into(),
            ]
        }
    }

    pub fn setup_tray(callback: TrayCallback, icon_rgba: &[u8]) -> anyhow::Result<Box<dyn std::any::Any>> {
        let tray = HourlyTray {
            callback,
            icon_data: icon_rgba.to_vec(),
        };
        let handle = tray.spawn()?;
        Ok(Box::new(handle))
    }
}

#[cfg(not(target_os = "linux"))]
mod other {
    use super::{TrayEvent, TrayCallback};
    use tray_icon::{TrayIconBuilder, menu::{Menu, MenuItem, MenuEvent}, Icon};

    pub fn setup_tray(callback: TrayCallback, icon_rgba: &[u8]) -> anyhow::Result<Box<dyn std::any::Any>> {
        let tray_menu = Menu::new();
        let config_i = MenuItem::new("Settings", true, None);
        let quit_i = MenuItem::new("Quit", true, None);
        tray_menu.append(&config_i)?;
        tray_menu.append(&quit_i)?;

        let icon = Icon::from_rgba(icon_rgba.to_vec(), 32, 32)?;

        let tray_icon = TrayIconBuilder::new()
            .with_menu(Box::new(tray_menu))
            .with_tooltip("Hourly Chime")
            .with_icon(icon)
            .build()?;

        // Spawn a thread to forward events
        let config_id = config_i.id().clone();
        let quit_id = quit_i.id().clone();
        
        std::thread::spawn(move || {
            let menu_channel = MenuEvent::receiver();
            while let Ok(event) = menu_channel.recv() {
                if event.id == config_id {
                    callback(TrayEvent::Settings);
                } else if event.id == quit_id {
                    callback(TrayEvent::Quit);
                }
            }
        });

        Ok(Box::new(tray_icon))
    }
}
