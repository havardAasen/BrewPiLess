FROM debian:bookworm

RUN apt-get update && apt-get install --yes --no-install-recommends \
    build-essential \
    curl \
    git \
    npm \
    python3-venv \
    ruby-dev \
    xxd \
    && rm -rf /var/lib/apt/lists/*

RUN  curl -fLo /etc/udev/rules.d/99-platformio-udev.rules --create-dir https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules

RUN python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"
