// Taken from https://stackoverflow.com/a/11814544/4447365

#include <stdlib.h>
#include <stdio.h>

FILE* open_or_exit(const char* fname, const char* mode)
{
  FILE* f = fopen(fname, mode);
  if (f == NULL) {
    perror(fname);
    exit(EXIT_FAILURE);
  }
  return f;
}

int main(int argc, char** argv)
{
  if (argc < 3) {
    fprintf(stderr, "USAGE: %s {sym} {rsrc}\n\n"
        "  Creates {sym}.cpp from the contents of {rsrc}\n",
        argv[0]);
    return EXIT_FAILURE;
  }

  const char* sym = argv[1];
  FILE* in = open_or_exit(argv[2], "r");

  char symfile[256];
  snprintf(symfile, sizeof(symfile), "%s.cpp", sym);

  FILE* out = open_or_exit(symfile,"w");
  fprintf(out, "namespace lightstep {\n");
  fprintf(out, "extern const unsigned char %s[];\n", sym);
  fprintf(out, "extern const int %s_size;\n", sym);
  fprintf(out, "const unsigned char %s[] = {\n", sym);

  unsigned char buf[256];
  size_t nread = 0;
  size_t linecount = 0;
  do {
    nread = fread(buf, 1, sizeof(buf), in);
    size_t i;
    for (i=0; i < nread; i++) {
      fprintf(out, "0x%02x, ", buf[i]);
      if (++linecount == 10) { fprintf(out, "\n"); linecount = 0; }
    }
  } while (nread > 0);
  if (linecount > 0) fprintf(out, "\n");
  fprintf(out, "};\n");
  fprintf(out, "const int %s_size = static_cast<int>(sizeof(%s));\n\n",sym,sym);
  fprintf(out, "} //namespace lightstep\n");

  fclose(in);
  fclose(out);

  return EXIT_SUCCESS;
}
