FROM ubuntu:22.04

# 安装常用基础工具
RUN apt-get update && apt-get install -y \
    wget curl git build-essential software-properties-common sudo \
    && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]
