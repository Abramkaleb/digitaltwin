// Mendapatkan referensi elemen-elemen HTML
const suhuValue = document.getElementById('suhu-value');
const rpmValue = document.getElementById('rpm-value');
const fuelValue = document.getElementById('fuel-value');

// Fungsi untuk memperbarui nilai sensor
function updateSensorValues(suhu, rpm, fuel) {
  suhuValue.textContent = suhu + '°C';
  rpmValue.textContent = rpm + ' RPM';
  fuelValue.textContent = fuel + '%';
}

// Contoh pembaruan data sensor (dapat diganti dengan data aktual dari Arduino)
const suhu = 28;
const rpm = 2500;
const fuel = 75;

// Memanggil fungsi updateSensorValues dengan data aktual
updateSensorValues(suhu, rpm, fuel);
