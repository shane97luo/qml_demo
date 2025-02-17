#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include <QImage>
#include <QObject>
#include <QUrl>

extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class VideoDecoder : public QObject {
  Q_OBJECT
public:
  explicit VideoDecoder(QObject *parent = nullptr);

  ~VideoDecoder();

  Q_INVOKABLE bool loadVideo(const QUrl &fileUrl); // 支持QML调用

  Q_INVOKABLE QImage getFrame(qint64 timestampMs);

  Q_INVOKABLE qint64 duration() const { return m_duration; }

  QUrl source() const { return m_source; }

signals:
  void sourceChanged();

  void loaded(qint64 durationMs);

  void error(const QString &message);

private:
  bool initCodec();

  void cleanup();

  QImage convertFrame(AVFrame *frame);

  AVFormatContext *m_formatCtx = nullptr;
  AVCodecContext *m_codecCtx = nullptr;
  SwsContext *m_swsCtx = nullptr;
  int m_videoStream = -1;
  qint64 m_duration = 0;
  QUrl m_source;
};

#endif // VIDEO_DECODER_H
