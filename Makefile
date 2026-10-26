
TARGET ?= qemu

# Reglas de compilacion: all, kernel, userland, image.
# Deben ejecutarse dentro de la imagen provista por la catedra (ver reglas docker-*).
all:  bootloader kernel userland image

bootloader:
	cd Bootloader; make all

kernel:
	cd Kernel; make all

userland:
	cd Userland; make all

image: kernel bootloader userland
	$(MAKE) -C Image TARGET=$(TARGET) all

# Compila usando el buddy system como memory manager (por defecto first fit)
buddy:
	$(MAKE) all MM=buddy

# Convenience targets for explicit images
image-qemu: TARGET=qemu
image-qemu: image

image-vbox: TARGET=vbox
image-vbox: image

image-usb: TARGET=usb
image-usb: image

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

# ---------------------------------------------------------------------------
# Entorno de compilacion (imagen de la catedra) y ejecucion.
# Estas reglas se usan FUERA del contenedor, en el host.
# ---------------------------------------------------------------------------
DOCKER_IMAGE = agodio/itba-so-multiarch:3.1
CONTAINER    = TPE_SO
IMAGE_FILE   = Image/x64BareBonesImage.qcow2
QEMU         = qemu-system-x86_64
QEMU_FLAGS  ?= -m 512

# Descarga la imagen de la catedra
docker-pull:
	docker pull $(DOCKER_IMAGE)

# Inicia el contenedor (lo crea si no existe) montando el repo en /root
docker-start:
	@if [ -n "$$(docker ps -aq -f name=^$(CONTAINER)$$)" ]; then \
		docker start $(CONTAINER); \
	else \
		docker run -d --name $(CONTAINER) -v "$(CURDIR)":/root -w /root \
			--security-opt seccomp:unconfined -ti $(DOCKER_IMAGE); \
	fi

# Detiene el contenedor
docker-stop:
	docker stop $(CONTAINER)

# Elimina el contenedor
docker-rm:
	-docker rm -f $(CONTAINER)

# Compila dentro del contenedor (equivale a "make all" dentro de la imagen)
docker-build: docker-start
	docker exec -t $(CONTAINER) make -C /root all

# Compila con el memory manager indicado: make docker-build-mm MM=buddy
docker-build-mm: docker-start
	docker exec -t $(CONTAINER) make -C /root all MM=$(MM)

# Limpia dentro del contenedor
docker-clean: docker-start
	docker exec -t $(CONTAINER) make -C /root clean

# Abre una shell dentro del contenedor
docker-shell: docker-start
	docker exec -ti $(CONTAINER) bash

# Inicia qemu con la imagen generada (en el host, no en el contenedor)
run:
	$(QEMU) -hda $(IMAGE_FILE) $(QEMU_FLAGS)

.PHONY: bootloader image collections kernel userland all clean buddy \
	docker-pull docker-start docker-stop docker-rm docker-build docker-build-mm \
	docker-clean docker-shell run
