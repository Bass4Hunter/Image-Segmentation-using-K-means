# Image-Segmentation-using-K-means

## Configuracion LAN

### Hamachi
Se puede lograr una LAN por medio de Hamachi para conectar computadoras remotamente.

* Descargar e instalar hamachi mediante el siguiente enlace

https://www.vpn.net/linux


* Luego por medio de la terminal ejecutamos el siguiente comando para registrarnos

``sudo hamachi login``

* Para unirnos a la red LAN

``sudo hamachi join vpn-paralela paralela``

* Para mostrar todos los nodos conectados a la red LAN

``sudo hamachi list``

## MPICH Cluster

### Definiendo hostnames

Seguimos el siguiente tutorial para generarlo

https://help.ubuntu.com/community/MpichCluster

* Editar el documento host para todos los nodos y el servidor

``sudo gedit /etc/hosts ``

* Con las siguientes lineas donde el ip hamachi ub0 sera el server y los demas seran los nodos

```
127.0.0.1 localhost
25.8.95.75 ub0
25.11.41.10 ub1
```

### Instalando NFS

* En el **servidor**

``sudo apt-get install nfs-server ``

* En el **nodos**

``sudo apt-get install nfs-client ``


### Carpeta compartida

* En todos los **nodos y servidor** se crea la carpeta *mirror*

``sudo mkdir /mirror``

* Y se le asigna permisos

``sudo chown mpiu /mirror ``

* En el **servidor** compartimos el contenido de esta carpeta ubicada en el nodo maestro con todos los demás nodos

``echo "/mirror *(rw,sync)" | sudo tee -a /etc/exports``

* Luego reinicie el servicio nfs en el **servidor**

``sudo service nfs-kernel-server restart ``

### Montaje en nodos

* En los **nodos** montar la carpeta mirror

``sudo mount ub0:/mirror /mirror ``

* Y dentro de */etc/fstab* cambiar fstab para montarlo en cada arranque 

``ub0:/mirror    /mirror    nfs ``

### Usuario MPI

* Crear usuario en todos los **nodos y servidor** con la contraseña ``paralela``

``sudo adduser mpiu ``

* En los **nodos** desmontar la carpeta

``sudo umount -f -l /mirror ``

* Y montarla dentro del nuevo usuario

``sudo mount -a ``

* En los **nodos** instalar OpenSSH Server

``sudo apt-get install openssh-server ``

* Ingresar al usuario nuevo, la contraseña es ``paralela``

``su - mpiu `` 

* En el **servidor** se genera un par de claves RSA para mpiu

``ssh-keygen -t rsa `` 

* Y agregamos esta clave a las claves autorizadas

``cd .ssh ``

``cat id_rsa.pub >> authorized_keys ``

* El **servidor** debe hacer eso para cada cliente

``ssh-copy-id {USUARIO ejmp ub1}``

* Para comprabar si se realizo correctamente dentro del servidor se puede ejecutar

``ssh {USUARIO} hostname``

* Para cambiar a otro usuario

``ssh {USUARIO}``

### Instalaciones

* En todos **nodos y servidor** se instala GCC y MPICH

``sudo apt-get install build-essential ``

``sudo apt-get install mpich``

* Y se verifica la ruta de mpich sea la misma para todos los nodos y servidor

``which mpiexec ``

``which mpirun ``

* En el **servidor** dentro de la carpeta mirror se agrego el archivo

``vim  machinefile``

* Con los nombres de los nodos seguidos de dos puntos y varios procesos para generar

```
ub1:2
ub0    
```

## Comandos para compilar y ejecutar

* Para compilarlo

``mpic++ -fopenmp -o main main.cpp Worker.cpp jpeg.cpp FileManager.cpp -ljpeg``

* Para ejecutarlo de manera local

``mpirun -np {CANTIDAD PROCESADORES} ./main ``

* Para ejecutarlo con los nodos

``mpirun -np {CANTIDAD PROCESADORES} -f ../machinefile ./main ``


## Limitaciones y sugerencias

* Asegurar que en todos los nodos y servidor se encunetre la misma version mpich
* Generar claves dentro del servidor ya que puede resultar un error al momento de ejecutar el codigo si no son generadas 
* El mpich debe estar instalado en la misma ruta, puede verificar con ``which mpiexec`` y ``which mpirun``
