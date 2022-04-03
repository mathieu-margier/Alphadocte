FROM debian:11
RUN apt-get update && \
# prevent image building stuck when asking tz data
    DEBIAN_FRONTEND=noninteractive \
    TZ_DATA=Etc/Utc \
    apt-get install -y build-essential \
                       git \
                       cmake dpkg-dev file \
                       libboost-dev
