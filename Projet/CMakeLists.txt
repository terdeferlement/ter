cmake_minimum_required( VERSION 3.10 )

project( PG201_TP1 VERSION 2023 )

set( CXX_STANDARD_REQUIRED On )
set( CXX_STANDARD 11 ) # C++11 est suffisant, ou 17/20 selon besoin

# Votre chemin vers Eigen
# set( LIBRARY_PATH_EIGEN "/Users/ethanlatouche/Downloads/DM1/test_Eigen/eigen-3.4.0/" )
#set( LIBRARY_PATH_EIGEN "/net/netud/m/elatouche001/TPCPP/TP3/test_Eigen/eigen/" )


# Inclusion des fichiers d'en-tête (headers) d'Eigen pour que le compilateur les trouve
include_directories( ${LIBRARY_PATH_EIGEN} )

# Si vous compilez DANS le dossier build (comme expliqué étape 1),
# cette ligne n'est pas nécessaire, mais on peut la garder pour forcer la sortie.
# Attention : si vous êtes déjà dans build, l'exécutable sera dans build/build/
# Je vous conseille de commenter cette ligne si vous suivez la méthode "mkdir build" manuelle :
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "build/" ) 

set( CMAKE_CXX_FLAGS "-Wall" )

# Options de build
if(NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE "Debug" )
endif()

set( TARGET_NAME "projet_${CMAKE_BUILD_TYPE}" )

# 1. Lister vos fichiers sources (.cpp)
# Remplacez main.cpp et SaintVenant.cpp par VOS fichiers
set( PROJECT_COMPILATION_FILE_LIST src/main.cpp src/SaintVenant.cpp )

# Précise que l'exécutable sera à assembler avec ces fichiers compilés.
add_executable( ${TARGET_NAME} ${PROJECT_COMPILATION_FILE_LIST} )

# Ajoute les répertoires des bibliothèques liées, ici Eigen.
target_include_directories( ${TARGET_NAME} PUBLIC "${LIBRARY_PATH_EIGEN}" )

# Fin des options spécifiques à ce projet.
##################

#################
## Rappels et notes
#
# - Pour générer le fichier 'Makefile', lancez "cmake CMakeLists.txt".
# - Pour compiler votre code, lancez simplement "make".
# - Pour voir en détail les commandes de compilations qu'exécute "make", lancez "make VERBOSE=1".
# - Pour nettoyer votre répertoire de compilation, lancez "make clean".
#################
# ==========================================