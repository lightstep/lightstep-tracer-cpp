namespace lightstep {
extern const unsigned char default_ssl_roots_pem[];
extern const int default_ssl_roots_pem_size;

// Add a single element to appease -Wzero-length-array
const unsigned char default_ssl_roots_pem[] = {'\0'};
const int default_ssl_roots_pem_size = 0;
}  // namespace lightstep
