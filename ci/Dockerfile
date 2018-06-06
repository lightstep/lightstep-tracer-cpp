FROM ubuntu:18.04

WORKDIR /3rd_party

ADD setup_build_environment.sh /3rd_party
ADD install_opentracing.sh /3rd_party
ADD install_grpc.sh /3rd_party
ADD install_clang.sh /3rd_party

RUN /3rd_party/setup_build_environment.sh \
 && /3rd_party/install_opentracing.sh \
 && /3rd_party/install_grpc.sh \
 && /3rd_party/install_clang.sh
