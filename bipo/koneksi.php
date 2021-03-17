<?php
$host ='localhost';
$user ='root';
$pas ='MRS.banten3d98';
$database='db_bipo';

$konek = mysqli_connect($host,$user,$pas,$database);

if (!$konek)
{
	echo "koneksi ke MYSQL gagal....";
}
?>