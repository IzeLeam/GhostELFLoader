FROM gcc:14.2

RUN apt update &&\
    apt install -y binutils-dev clang-19 clang-tidy-19

COPY --chmod=555 ./scripts/* /root/scripts-isos/
COPY ./Makefile /root/Makefile.internal
COPY ./include /root/include
ENV PATH=/root/scripts-isos:$PATH \
    INTERNAL_MAKEFILE="/root/Makefile.internal"

