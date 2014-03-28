#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>

#include <memory>

class Controller : public QObject
{
  Q_OBJECT
public:
  explicit Controller(QObject *parent = 0);

  ~Controller();

  /**
   * Activates the Gui dialog.
   */
  void activate();

signals:

private slots:
  void onLoadImage(const QString& imageFolder);
  void onRequestImage(int index);

private:
  class Pimpl;
  std::unique_ptr<Pimpl> _pimpl;

};

#endif // CONTROLLER_H
