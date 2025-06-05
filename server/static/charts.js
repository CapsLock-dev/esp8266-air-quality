var time_range = document.getElementById('timeRange').value

const tempChart = new Chart(
    document.getElementById('tempChart'),
    { type: 'line', data: { datasets: [{ label: 'Temperature' }] } }
);

const humChart = new Chart(
    document.getElementById('humChart'),
    { type: 'line', data: { datasets: [{ label: 'Humidity' }] } }
);

const eco2Chart = new Chart(
    document.getElementById('eco2Chart'),
    { type: 'line', data: { datasets: [{ label: 'CO2' }] } }
);

const tvocChart = new Chart(
    document.getElementById('tvocChart'),
    { type: 'line', data: { datasets: [{ label: 'TVOC' }] } }
);

async function updateDashboard() {
    try {
        const response = await fetch(`/api/sensor-data?hours=${time_range}`);
        const data = await response.json();
        
        if (data.length === 0) return;
        
        const latest = data[data.length - 1];
        document.getElementById('current-temp').textContent = `${latest.temp} °C`;
        document.getElementById('current-hum').textContent = `${latest.hum} %`;
        document.getElementById('current-eco2').textContent = `${latest.eco2} ppm`;
        document.getElementById('current-tvoc').textContent = `${latest.tvoc} ppb`;
		aqi_texts = ["None", "Perfect", "Moderate", "May be unhealthy", "Unhealthy", "Very unhealthy"]
        document.getElementById('current-aqi').textContent = `${aqi_texts[latest.aqi]} (${latest.aqi})`;
        
        updateChart(tempChart, data, 'temp', 'Temperature (°C)');
        updateChart(humChart, data, 'hum', 'Humidity (%)');
        updateChart(eco2Chart, data, 'eco2', 'CO2 (ppm)');
        updateChart(tvocChart, data, 'tvoc', 'TVOC (ppb)');
        
    } catch (error) {
        console.error('Failed to update dashboard:', error);
    }
}

function updateChart(chart, data, field, label) {
	const timeLabels = data.map(item => item.time);

	console.log(timeLabels)
    chart.data.labels = timeLabels;
    chart.data.datasets[0].data = data.map(item => item[field]);
    chart.data.datasets[0].label = label;
    chart.update();
}

document.getElementById('timeRange').addEventListener('change', (e) => {
	time_range = parseInt(e.target.value)
    updateDashboard();
});

updateDashboard();
setInterval(() => updateDashboard(), 60000);
