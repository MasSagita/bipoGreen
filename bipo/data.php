<?php
include "koneksi.php";
?>

<?php
$x_tanggal  = mysqli_query($konek, 'SELECT reading_time FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 50) Var1 ORDER BY ID ASC');
$x_tanggal2  = mysqli_query($konek, 'SELECT reading_time FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 50) Var1 ORDER BY ID ASC');
$x_tanggal3  = mysqli_query($konek, 'SELECT reading_time FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 50) Var1 ORDER BY ID ASC');

$y_ph   = mysqli_query($konek, 'SELECT value1 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 60) Var1 ORDER BY ID ASC');
$y_ds   = mysqli_query($konek, 'SELECT value2 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 60) Var1 ORDER BY ID ASC');
$y_dhtT = mysqli_query($konek, 'SELECT value3 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 60) Var1 ORDER BY ID ASC');
$y_bar  = mysqli_query($konek, 'SELECT value4 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 20) Var1 ORDER BY ID ASC');
$y_psi  = mysqli_query($konek, 'SELECT value5 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 20) Var1 ORDER BY ID ASC');
$y_dhtH = mysqli_query($konek, 'SELECT value6 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 20) Var1 ORDER BY ID ASC');
$y_ch4  = mysqli_query($konek, 'SELECT value7 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 20) Var1 ORDER BY ID ASC');
$y_co2  = mysqli_query($konek, 'SELECT value8 FROM ( SELECT * FROM sensor ORDER BY id DESC LIMIT 20) Var1 ORDER BY ID ASC');
?>

<div class="panel panel-primary">
  <div class="panel-heading">
    <h3 class="panel-title">
      <center>BiPo Grafik
    </h3>
  </div>

  <div class="panel-body">
    <canvas id="myChart2"></canvas>

    <script>
      var canvas = document.getElementById('myChart2');
      var data = {
        labels: [<?php while ($b = mysqli_fetch_array($x_tanggal2)) {
                    echo '"' . $b['reading_time'] . '",';
                  } ?>],
        datasets: [{
          label: "Nilai ph",
          fill: false,
          lineTension: 0.1,
          backgroundColor: "rgb(0, 176, 176)",
          borderColor: "rgb(0, 176, 176)",
          borderCapStyle: 'butt',
          borderDash: [],
          borderDashOffset: 0.0,
          borderJoinStyle: 'miter',
          pointBorderColor: "rgba(0, 176, 176, .7)",
          pointBackgroundColor: "rgb(0, 176, 176)",
          pointBorderWidth: 1,
          pointHoverRadius: 3,
          pointHoverBackgroundColor: "rgba(0, 176, 176, .7)",
          pointHoverBorderColor: "rgba(0, 176, 176, .7)",
          pointHoverBorderWidth: 2,
          pointRadius: 3,
          pointHitRadius: 10,
          data: [<?php while ($b = mysqli_fetch_array($y_ph)) {
                    echo  $b['value1'] . ',';
                  } ?>],
        }]
      };

      var option = {
        scales: {
          yAxes: [{
            display: true,
            ticks: {
              beginAtZero: true,
              steps: 1,
              stepValue: 0.1,
              max: 14
            }

          }]
        },
        title: {
          display: true,
          text: 'Sensor pH'
        },
        showLines: true,
        animation: {
          duration: 0
        }
      };

      var myLineChart = Chart.Line(canvas, {
        data: data,
        options: option
      });
    </script>
  </div>

  <div class="panel-body">
    <canvas id="myChart"></canvas>
    <script>
      var canvas = document.getElementById('myChart');
      var data = {
        labels: [<?php while ($b = mysqli_fetch_array($x_tanggal)) {
                    echo '"' . $b['reading_time'] . '",';
                  } ?>],
        datasets: [{
            label: "DHT11",
            fill: false,
            lineTension: 0.01,
            backgroundColor: "rgb(230, 230, 0)",
            borderColor: "rgb(230, 230, 0)",
            borderCapStyle: 'round',
            borderDash: [],
            borderDashOffset: 0.0,
            borderJoinStyle: 'miter',
            pointBorderColor: "rgba(230, 230, 0, .7)",
            pointBackgroundColor: "rgba(230, 230, 0, .7)",
            pointBorderWidth: 1,
            pointHoverRadius: 3,
            pointHoverBackgroundColor: "rgba(230, 230, 0, .7)",
            pointHoverBorderColor: "rgba(230, 230, 0, .7)",
            pointHoverBorderWidth: 2,
            pointRadius: 3,
            pointHitRadius: 10,
            data: [<?php while ($b = mysqli_fetch_array($y_ds)) {
                      echo  $b['value2'] . ',';
                    } ?>],
          },
          {
            label: "DS18B20",
            fill: false,
            lineTension: 0.01,
            backgroundColor: "rgb(167, 0, 0)",
            borderColor: "rgb(167, 0, 0)",
            borderCapStyle: 'round',
            borderDash: [],
            borderDashOffset: 0.0,
            borderJoinStyle: 'miter',
            pointBorderColor: "rgba(167, 0, 0, .7)",
            pointBackgroundColor: "rgb(167, 0, 0)",
            pointBorderWidth: 1,
            pointHoverRadius: 3,
            pointHoverBackgroundColor: "rgba(167, 0, 0, .7)",
            pointHoverBorderColor: "rgba(167, 0, 0, .7)",
            pointHoverBorderWidth: 2,
            pointRadius: 3,
            pointHitRadius: 10,
            data: [<?php while ($b = mysqli_fetch_array($y_dhtT)) {
                      echo  $b['value3'] . ',';
                    } ?>],
          }
        ]
      };

      var option = {
        showLines: true,
        title: {
          display: true,
          text: 'Sensor Suhu'
        },
        animation: {
          duration: 0
        }
      };
      var myLineChart = Chart.Line(canvas, {
        data: data,
        options: option
      });
    </script>
  </div>
</div>

<div class="panel panel-primary">
  <div class="panel-heading">
    <h3 class="panel-title">
      <center>BiPo Tabel
    </h3>
  </div>
  <div class="panel-body">
    <table class="table table-bordered table-striped">
      <thead>
        <tr>
          <th class='text-center'>No</th>
          <th class='text-center'>Waktu</th>
          <th class='text-center'>ph Tinja</th>
          <th class='text-center'>Suhu Tangki (C)</th>
          <th class='text-center'>Suhu Luar (C)</th>
          <th class='text-center'>Tekanan (bar)</th>
          <th class='text-center'>Tekanan (psi)</th>
          <th class='text-center'>Kelembapan (%)</th>
          <th class='text-center'>Metana</th>
          <th class='text-center'>Karbondioksida</th>
        </tr>
      </thead>

      <tbody>
        <?php

        $sqlAdmin = mysqli_query($konek, "SELECT id,reading_time,value1,value2,value3,value4,value5,value6,value7,value8 FROM sensor ORDER BY ID DESC LIMIT 0,100");
        while ($data = mysqli_fetch_array($sqlAdmin)) {
          echo "<tr >
                <td><center>$data[id]</center></td>
                <td><center>$data[reading_time]</center></td> 
                <td><center>$data[value1]</td>
                <td><center>$data[value2]</td>
                <td><center>$data[value3]</td>
                <td><center>$data[value4]</td>
                <td><center>$data[value5]</td>
                <td><center>$data[value6]</td>
                <td><center>$data[value7]</td>
                <td><center>$data[value8]</td>
              </tr>";
        }
        ?>
      </tbody>
    </table>
  </div>
</div>