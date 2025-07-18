#include "trayicon.hh"
#include "menu.hh"
#include "settingsdialog.hh"
#include <QSound>
#include "menu.hh"
#include "icons.hh"

TrayIcon::TrayIcon(Application *app)
    : QSystemTrayIcon(), _app(app)
{
  setIcon(IconProvider::get(IconProvider::TRAY_ICON_INACTIVE));
  Menu *menu = new Menu(_app);
  setContextMenu(menu);

	connect(_app->spots(), SIGNAL(connected()), this, SLOT(onClusterConnected()));
  connect(_app->spots(), SIGNAL(disconnected()), this, SLOT(onClusterDisconnected()));

  _spotWindow = new SpotWindow(_app->spots());
  _spotWindow->setVisible(false);

  _mapView = new SelfSpotMapView(Settings().locator(), *_app);
  _mapView->setVisible(false);

  connect(menu->showSpotWindow(), SIGNAL(triggered()), _spotWindow, SLOT(show()));
  connect(menu->showMapWindow(), SIGNAL(triggered()), _mapView, SLOT(show()));
  connect(menu->showSettings(), SIGNAL(triggered()), this, SLOT(onShowSettings()));

  _popup = new PopUp();
  _popup->setProperty("popupDuration", 5000);
  connect(_app->spots(), SIGNAL(newDXCC(Spot)), this, SLOT(onNewDXCC(Spot)));
  connect(_app->spots(), SIGNAL(newBand(Spot)), this, SLOT(onNewBand(Spot)));
  connect(_app->spots(), SIGNAL(newMembership(Spot,Membership)),
          this, SLOT(onNewMembership(Spot,Membership)));
  connect(_app->spots(), SIGNAL(newFriend(Spot)), this, SLOT(onNewFriend(Spot)));
}

TrayIcon::~TrayIcon()
{
  _popup->deleteLater();
  _spotWindow->deleteLater();
}

void
TrayIcon::onClusterConnected() {
  setIcon(IconProvider::get(IconProvider::TRAY_ICON));
}

void
TrayIcon::onClusterDisconnected() {
  setIcon(IconProvider::get(IconProvider::TRAY_ICON_INACTIVE));
  _popup->setPopupText(tr("Disconnected from cluster!"));
}

void
TrayIcon::onShowSettings() {
  SettingsDialog dialog;
  if (QDialog::Accepted == dialog.exec()) {
    Settings settings;
    _app->spots()->friends() = settings.friends();
    _app->spots()->setLocator(settings.locator());
    _app->spots()->setCall(settings.call());
  }
}

void
TrayIcon::onNewDXCC(const Spot &spot) {
  if (! Settings().notifyOnNewDXCC())
    return;

  qDebug() << "Notify new DXCC...";
  _popup->setPopupText(tr("New DXCC <b>%1</b> on <b>%2kHz</b> (%3dB, %4WPM)").arg(spot.full_call)
                       .arg(spot.freq).arg(spot.db).arg(spot.wpm));
  _popup->show();

  if (Settings().notificationSoundEnabled())
    QSound::play(":/sound/single_chime.wav");
}

void
TrayIcon::onNewBand(const Spot &spot) {
  if (! Settings().notifyOnNewBand())
    return;

  qDebug() << "Notify new Band...";
  _popup->setPopupText(tr("New Band <b>%1</b> on <b>%2kHz</b> (%3dB, %4WPM)").arg(spot.full_call)
                       .arg(spot.freq).arg(spot.db).arg(spot.wpm));
  _popup->show();

  if (Settings().notificationSoundEnabled())
    QSound::play(":/sound/single_chime.wav");
}

void
TrayIcon::onNewMembership(const Spot &spot, const Membership &memb) {
  if (! (Settings().notifyOnNewMembership() & memb).any())
    return;

  Membership notify = Settings().notifyOnNewMembership() & memb;
  QString memberships = notify.names().join(", ").toUpper();
  qDebug() << "Notify new" << memberships << "..." << spot.full_call;
  _popup->setPopupText(tr("New %1 <b>%2</b> on <b>%3kHz</b> (%4dB, %5WPM)")
                       .arg(memberships).arg(spot.full_call)
                       .arg(spot.freq).arg(spot.db).arg(spot.wpm));
  _popup->show();

  if (Settings().notificationSoundEnabled())
    QSound::play(":/sound/single_chime.wav");
}

void
TrayIcon::onNewFriend(const Spot &spot) {
  if (! Settings().notifyOnNewFriend())
    return;
  qDebug() << "Notify Friend" << spot.full_call;
  Friend fri = _app->spots()->friends()[spot.full_call];
  if (fri.name().isEmpty()) {
    _popup->setPopupText(tr("Friend <b>%1</b> on <b>%2kHz</b> (%3dB, %4WPM)").arg(spot.full_call)
                         .arg(spot.freq).arg(spot.db).arg(spot.wpm));
  } else {
    _popup->setPopupText(tr("%1 <b>%2</b> on <b>%3kHz</b> (%4dB, %5WPM)")
                         .arg(fri.name()).arg(spot.full_call)
                         .arg(spot.freq).arg(spot.db).arg(spot.wpm));

  }
  _popup->show();

  if (Settings().notificationSoundEnabled())
    QSound::play(":/sound/single_chime.wav");
}

