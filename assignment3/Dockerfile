FROM nvidia/opencl

RUN apt-get update

RUN apt-get install git build-essential g++ gcc opencl-headers python3 -y

RUN mkdir /wd

RUN printf '%s\n%s\n' "color_prompt=yes" "$(cat ~/.bashrc)" >~/.bashrc

WORKDIR /wd

COPY setup-inside-container.sh setup-inside-container.sh

RUN chmod +x setup-inside-container.sh

ENTRYPOINT /bin/bash ./setup-inside-container.sh && /bin/bash