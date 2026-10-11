#!/bin/bash
set -euo pipefail

# Uso: ./run.sh [TARGET]
# TARGET puede ser: qemu (default), vbox, usb

TARGET_ARG="${1:-${TARGET_ARG:-qemu}}"

if [ "${TARGET_ARG}" = "qemu" ]; then
	if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
		echo "qemu-system-x86_64 no encontrado. Instale QEMU para ejecutar la imagen." >&2
		exit 1
	fi

	# Preferir qcow2 (más ligera para qemu), fallback a img
	if [ -f Image/x64BareBonesImage.qcow2 ]; then
		IMG="Image/x64BareBonesImage.qcow2"
		FORMAT="qcow2"
	elif [ -f Image/x64BareBonesImage.img ]; then
		IMG="Image/x64BareBonesImage.img"
		FORMAT="raw"
	else
		echo "No se encontró imagen QEMU en Image/x64BareBonesImage.qcow2 ni .img" >&2
		exit 1
	fi

	echo "Arrancando QEMU con ${IMG}..."

	# Intentamos habilitar audio de PC speaker si QEMU soporta pcspk-audiodev.
	if qemu-system-x86_64 --version >/dev/null 2>&1; then
		# Primera opción: audiodev + pcspk
		if qemu-system-x86_64 -drive file="${IMG}",format=${FORMAT},if=ide -m 512 -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0 >/dev/null 2>&1; then
			# el comando anterior fue solo para probar si qemu acepta las opciones; relanzamos para que el usuario lo vea
			qemu-system-x86_64 -drive file="${IMG}",format=${FORMAT},if=ide -m 512 -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0
			exit 0
		else
			# Fallback: arrancar sin opciones de audio avanzadas
			qemu-system-x86_64 -drive file="${IMG}",format=${FORMAT},if=ide -m 512
			exit 0
		fi
	else
		qemu-system-x86_64 -drive file="${IMG}",format=${FORMAT},if=ide -m 512
		exit 0
	fi

elif [ "${TARGET_ARG}" = "vbox" ]; then
	if [ -f Image/x64BareBonesImage.vmdk ]; then
		echo "Imagen para VirtualBox lista: Image/x64BareBonesImage.vmdk"
		echo "Cree una VM en VirtualBox y adjunte este VMDK como disco."
	else
		echo "No se encontró Image/x64BareBonesImage.vmdk. Genere la imagen con ./compile.sh TARGET=vbox primero." >&2
		exit 1
	fi

elif [ "${TARGET_ARG}" = "usb" ] || [ "${TARGET_ARG}" = "img" ]; then
	if [ -f Image/x64BareBonesImage.img ]; then
		echo "Imagen cruda para USB lista: Image/x64BareBonesImage.img"
		echo "Para grabarla en un pendrive (PELIGROSO), ejecute:" 
		echo "  sudo dd if=Image/x64BareBonesImage.img of=/dev/sdX bs=4M status=progress oflag=sync"
		echo "Reemplace /dev/sdX por su dispositivo USB."
	else
		echo "No se encontró Image/x64BareBonesImage.img. Genere la imagen con ./compile.sh TARGET=usb primero." >&2
		exit 1
	fi

else
	echo "TARGET desconocido: ${TARGET_ARG}. Opciones válidas: qemu, vbox, usb" >&2
	exit 1
fi
