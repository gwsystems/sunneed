# specify base image
FROM jupyter/tensorflow-notebook:dd2087c75645

# to run in complete isolation, copy the files (no write priv)
# COPY . .

# install dependencies
RUN pip3 install keras && \
    pip3 install git+https://github.com/jonlee48/google-images-download.git@patch-1 && \
    pip3 install seaborn
