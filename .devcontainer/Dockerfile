FROM debian:trixie

RUN \
    apt update \
    && apt install --yes --no-install-recommends \
        build-essential \
        curl \
        git \
        npm \
        pipx \
        python3-venv \
    && rm -rf \
        /tmp/* \
        /var/{cache,log}* \
        /var/lib/apt/lists/*

RUN  curl -fLo /etc/udev/rules.d/99-platformio-udev.rules --create-dirs https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules

RUN useradd -m brewpiless
USER brewpiless
ENV PATH="/home/brewpiless/.local/bin:${PATH}"

RUN pipx install platformio

RUN \
    platformio settings set enable_telemetry No \
    && platformio settings set check_platformio_interval 1000000
