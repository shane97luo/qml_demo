#include "gif_converter.h"
#include <QRgb>

ConverterWorker::ConverterWorker(VideoDecoder *decoder, QObject *parent)
    : QObject(parent), m_decoder(decoder) {}

void ConverterWorker::process(qint64 startMs, qint64 endMs,
                              const QString &path) {
  const int frameInterval = 100; // 10 FPS
}

GifConverter::GifConverter(QObject *parent) : QObject(parent) {
  ConverterWorker *worker = new ConverterWorker(m_decoder, nullptr);

  worker->moveToThread(&m_workerThread);
}

GifConverter::~GifConverter() {}

// void GifConverter::setFrameRange(qint64 startMs, qint64 endMs) {}

// void GifConverter::setOutputPath(const QString &path) {}

// void GifConverter::startConversion() {}
