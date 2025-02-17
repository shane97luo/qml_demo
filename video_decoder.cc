
#include "video_decoder.h"
#include <QDebug>

VideoDecoder::VideoDecoder(QObject *parent) : QObject(parent) {
  avformat_network_init();
}

VideoDecoder::~VideoDecoder() { cleanup(); }

bool VideoDecoder::loadVideo(const QUrl &fileUrl) {

  qDebug() << "file: " << fileUrl;

  m_source = fileUrl;

  const QString path = fileUrl.toLocalFile();
  if (avformat_open_input(&m_formatCtx, path.toUtf8().constData(), nullptr,
                          nullptr) != 0) {
    emit error("无法打开文件");
    return false;
  }

  // Find stream info
  if (avformat_find_stream_info(m_formatCtx, nullptr) < 0) {
    emit error("无法获取流信息");
    return false;
  }

  // Find video stream
  m_videoStream =
      av_find_best_stream(m_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (m_videoStream < 0) {
    emit error("未找到视频流");
    return false;
  }

  // Initialize codec
  if (!initCodec())
    return false;

  // Get duration
  AVStream *stream = m_formatCtx->streams[m_videoStream];
  m_duration = (stream->duration * av_q2d(stream->time_base)) * 1000;

  emit loaded(m_duration);
  emit sourceChanged();
  return true;
}

bool VideoDecoder::initCodec() {
  AVStream *stream = m_formatCtx->streams[m_videoStream];
  const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (!codec) {
    emit error("不支持的编解码器");
    return false;
  }

  m_codecCtx = avcodec_alloc_context3(codec);
  avcodec_parameters_to_context(m_codecCtx, stream->codecpar);

  if (avcodec_open2(m_codecCtx, codec, nullptr) < 0) {
    emit error("无法初始化解码器");
    return false;
  }

  // Init SWScale for color conversion
  m_swsCtx =
      sws_getContext(m_codecCtx->width, m_codecCtx->height, m_codecCtx->pix_fmt,
                     m_codecCtx->width, m_codecCtx->height, AV_PIX_FMT_RGB32,
                     SWS_BILINEAR, nullptr, nullptr, nullptr);

  return true;
}

QImage VideoDecoder::getFrame(qint64 timestampMs) {
  if (!m_formatCtx || m_videoStream < 0)
    return QImage();

  // Seek to timestamp
  AVStream *stream = m_formatCtx->streams[m_videoStream];
  const qint64 targetPts = (timestampMs / 1000.0) / av_q2d(stream->time_base);

  if (av_seek_frame(m_formatCtx, m_videoStream, targetPts,
                    AVSEEK_FLAG_BACKWARD) < 0) {
    emit error("定位失败");
    return QImage();
  }

  // Read frames
  AVPacket packet;
  AVFrame *frame = av_frame_alloc();
  QImage result;

  while (av_read_frame(m_formatCtx, &packet) >= 0) {
    if (packet.stream_index == m_videoStream) {
      if (avcodec_send_packet(m_codecCtx, &packet) == 0) {
        while (avcodec_receive_frame(m_codecCtx, frame) == 0) {
          const qint64 frameTime =
              (frame->best_effort_timestamp * av_q2d(stream->time_base)) * 1000;
          if (frameTime >= timestampMs) {
            result = convertFrame(frame);
            av_packet_unref(&packet);
            av_frame_free(&frame);
            return result;
          }
        }
      }
    }
    av_packet_unref(&packet);
  }

  av_frame_free(&frame);
  return QImage();
}

QImage VideoDecoder::convertFrame(AVFrame *frame) {
  AVFrame *rgbFrame = av_frame_alloc();
  rgbFrame->format = AV_PIX_FMT_RGB32;
  rgbFrame->width = frame->width;
  rgbFrame->height = frame->height;
  av_frame_get_buffer(rgbFrame, 0);

  sws_scale(m_swsCtx, frame->data, frame->linesize, 0, frame->height,
            rgbFrame->data, rgbFrame->linesize);

  QImage img(rgbFrame->data[0], frame->width, frame->height,
             rgbFrame->linesize[0], QImage::Format_RGB32);

  av_frame_free(&rgbFrame);
  return img.copy();
}

void VideoDecoder::cleanup() {
  if (m_swsCtx)
    sws_freeContext(m_swsCtx);
  if (m_codecCtx)
    avcodec_free_context(&m_codecCtx);
  if (m_formatCtx)
    avformat_close_input(&m_formatCtx);

  m_swsCtx = nullptr;
  m_codecCtx = nullptr;
  m_formatCtx = nullptr;
  m_videoStream = -1;
  m_duration = 0;
}