document.addEventListener('DOMContentLoaded', function() {
  // Ambil elemen-elemen dengan ID yang sesuai
  var temperatureElement = document.getElementById('temperature');
  var rpmElement = document.getElementById('rpm');
  
  // Fungsi untuk memperbarui nilai suhu
  function updateTemperature(value) {
    temperatureElement.textContent = value + ' °C';
  }
  
  // Fungsi untuk memperbarui nilai RPM
  function updateRPM(value) {
    rpmElement.textContent = value + ' RPM';
  }
  
  // Menghubungkan ke Arduino atau sumber data lainnya
  // Gunakan teknik komunikasi seperti AJAX atau WebSocket
  // untuk memperbarui nilai suhu dan RPM secara periodik
  
  // Contoh penggunaan data statis
  var temperature = 25;
  var rpm = 1500;
  
  updateTemperature(temperature);
  updateRPM(rpm);
});
