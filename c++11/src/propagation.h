#include <opentracing/propagation.h>
#include <unordered_map>

namespace lightstep {
opentracing::expected<void> InjectSpanContext(
    std::ostream& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<void> InjectSpanContext(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> ExtractSpanContext(
    std::istream& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> ExtractSpanContext(
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage);
}  // namespace lightstep
