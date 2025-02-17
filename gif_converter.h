#ifndef GIF_CONVERTER_H
#define GIF_CONVERTER_H

#include "video_decoder.h"
#include <QImage>
#include <QObject>
#include <QString>
#include <QThread>
#include <QVector>

#include <gif_lib.h>

class ConverterWorker : public QObject {
  Q_OBJECT
public:
  explicit ConverterWorker(VideoDecoder *decoder, QObject *parent = nullptr);

public slots:
  void process(qint64 startMs, qint64 endMs, const QString &path);
signals:
  void progress(int percent);
  void finished();
  void error(QString message);

private:
  VideoDecoder *m_decoder;
};

class GifConverter : public QObject {
  Q_OBJECT

public:
  explicit GifConverter(QObject *parent = nullptr);

  ~GifConverter();

  void setDecoder(VideoDecoder *decoder) { m_decoder = decoder; }

public slots:
  // void convert(qint64 startMs, qint64 endMs, const QString &outputPath);

signals:
  void progress(int percent);
  void finished();
  void error(const QString &message);

private:
  struct FrameData {
    QImage image;
    int delay;
  };

  VideoDecoder *m_decoder = nullptr;
  QThread m_workerThread;
};

#endif // GIF_CONVERTER_H