#pragma once

/* #include <lightstep/spdlog/sinks/sink.h> */
/* #include <opentracing/string_view.h> */
/* #include <exception> */
/* #include <functional> */

/* namespace lightstep { */
/* class CustomLoggerSink : public spdlog::sinks::sink { */
/*  public: */
/*   explicit CustomLoggerSink( */
/*       std::function<void(opentracing::string_view)>&& sink) */
/*       : sink_{std::move(sink)} {} */

/*   void log(const spdlog::details::log_msg& message) override try { */
/*     sink_(opentracing::string_view{message.formatted.data(), */
/*                                    message.formatted.size()}); */
/*   } catch (const std::exception&) { */
/*     // Ignore if exception thrown */
/*   } */

/*   void flush() override {} */

/*  private: */
/*   std::function<void(opentracing::string_view)> sink_; */
/* }; */
/* }  // namespace lightstep */
