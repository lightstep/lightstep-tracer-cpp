#include <string>

const char base64bytes[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// See http://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
std::string base64_encode(const std::string &in) {

    std::string out;

    int val=0, valb=-6;
    for (unsigned char c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back(base64bytes[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back(base64bytes[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

std::string base64_decode(const std::string &in) {

    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T[base64bytes[i]] = i; 

    int val=0, valb=-8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}

int main() {
  try {

    // std::string data;
    // binary.SerializeToString(&data);
    // std::cerr << "ENCODED: " << base64_encode(data) << std::endl;

    // //std::string testdata = "ETo4qBGmBwYHGTZon1Du/8ZYIAEqEgoHY2hlY2tlZBIHYmFnZ2FnZQ==";
    // auto testdata = base64_encode(data);
    // envoystruct.Clear();
    // auto x = envoystruct.ParseFromString(base64_decode(testdata));
    // std::cerr << "DECODE: " << base64_decode(testdata) << std::endl;
    // std::cerr << "STRUCT: " << envoystruct.DebugString() << std::endl;
    // std::cerr << "X: " << x << std::endl;

    // lightstep::SpanContext envoy2 =
    //   lightstep::Tracer::Global().Extract(lightstep::CarrierFormat::EnvoyProtoCarrier,
    // 					  lightstep::envoy::ProtoReader(envoystruct));

    // std::cerr << "TID: " << envoy2.trace_id()
    // 	      << " SID: " << envoy2.span_id() << std::endl;

    // envoy2.ForeachBaggageItem([](const std::string& key, const std::string& value) {
    // 			     std::cerr << "BAGGAGE: " << key << ":" << value << std::endl;
    // 			     return true;
    // 				 });



  } catch (std::exception &e) {
    std::cerr << "Exception! " << e.what() << std::endl;
    return 1;
  }

  std::cerr << "Success!" << std::endl;
  return 0;
}
