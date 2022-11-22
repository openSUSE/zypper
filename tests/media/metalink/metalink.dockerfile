FROM opensuse/leap:15.4
ENV container docker

ENV LANG en_US.UTF-8

RUN zypper ar -f http://download.opensuse.org/repositories/openSUSE:infrastructure:MirrorCache/15.4 mc
RUN zypper --gpg-auto-import-keys ref

# install MirrorCache here to fetch all dependencies
RUN zypper -vvv -n install MirrorCache \
    vim postgresql postgresql-server curl sudo git-core wget tar m4 make \
    perl-Digest-MD4 tidy perl-DateTime-HiRes \
    perl-Inline-C gcc # dependencies for hashes calculation

WORKDIR /opt/project
ENV TZ UTC

# let pg initialize data dir in cache to save some time on every run
RUN sudo -u postgres /usr/share/postgresql/postgresql-script start && \
     sudo -u postgres createuser mirrorcache && \
     sudo -u postgres createdb mirrorcache && \
     sudo -u postgres /usr/share/postgresql/postgresql-script stop

COPY testfile /opt/project/testfile

ENV MIRRORCACHE_ROOT=/opt/project \
    MIRRORCACHE_PERMANENT_JOBS='' \
    MIRRORCACHE_HASHES_QUEUE=default \
    MIRRORCACHE_HASHES_COLLECT=1

RUN echo 100 # change this number to invalidate docker cache

RUN sudo -u postgres /usr/share/postgresql/postgresql-script start && \
    sudo -u mirrorcache -E /usr/share/mirrorcache/script/mirrorcache minion job -e folder_sync -a '["/"]' && \
    sudo -u mirrorcache -E /usr/share/mirrorcache/script/mirrorcache backstage run --oneshot && \
    ( sudo -u mirrorcache -E /usr/share/mirrorcache/script/mirrorcache daemon & ) && \
    sleep 5 && \
    curl -is 127.0.0.1:3000/download/testfile.meta4
