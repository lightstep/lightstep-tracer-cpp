#include <opentracing/propagation.h>
#include <unordered_map>

namespace lightstep {
opentracing::expected<void> inject_span_context(
    std::ostream& carrier, uint64_t trace_id, uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<void> inject_span_context(
    const opentracing::TextMapWriter& carrier, uint64_t trace_id,
    uint64_t span_id,
    const std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> extract_span_context(
    std::istream& carrier, uint64_t& trace_id, uint64_t& span_id,
    std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> extract_span_context(
    const opentracing::TextMapReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage);

opentracing::expected<bool> extract_span_context(
    const opentracing::HTTPHeadersReader& carrier, uint64_t& trace_id,
    uint64_t& span_id, std::unordered_map<std::string, std::string>& baggage);
}  // namespace lightstep
