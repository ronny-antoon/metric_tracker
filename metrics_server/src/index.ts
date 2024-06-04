import express, { Request, Response } from 'express';
import { InfluxDB, Point } from '@influxdata/influxdb-client';

// Initialize Express app
const app = express();
const port = 3000;

// Initialize InfluxDB client
const influxDB = new InfluxDB({ url: 'http://influxdb:8086', token: 'd0251d6942864581fcd39472b0de68023bca4ae937ed6c021a445a542322559e' });
const writeApi = influxDB.getWriteApi('my-org', 'my-bucket');

// Middleware to parse JSON bodies
app.use(express.json());

// Define interfaces for metrics payload
interface Metric {
  name: string;
  value: number;
}

interface Task {
  name: string;
  stack_free: number;
}

interface MetricsPayload {
  device_id: string;
  metrics?: Metric[];
  heap?: Metric[];
  tasks?: Task[];
}

// POST endpoint to receive metrics data
app.post('/metrics', (req: Request, res: Response) => {
  // Extract payload from request body
  const metricsPayload: MetricsPayload = req.body;

  // Log incoming payload for debugging
  console.info(req.body);

  // Validate the payload
  if (!metricsPayload.device_id || (!metricsPayload.metrics && !metricsPayload.heap && !metricsPayload.tasks)) {
    return res.status(400).send('Invalid payload');
  }

  // Get current timestamp and device ID
  const timestamp = new Date();
  const device_id = metricsPayload.device_id;

  // Process metrics if available
  if (metricsPayload.metrics) {
    metricsPayload.metrics.forEach(metric => {
      // Create a new data point for each metric
      const point = new Point('device_metrics')
        .tag('device_id', device_id)
        .timestamp(timestamp)
        .intField(metric.name, metric.value);

      // Write the data point to InfluxDB
      writeApi.writePoint(point);
    });
  }

  // Process heap data if available
  if (metricsPayload.heap) {
    metricsPayload.heap.forEach(heap => {
      // Create a new data point for each heap metric
      const point = new Point('device_heap')
        .tag('device_id', device_id)
        .timestamp(timestamp)
        .intField(heap.name, heap.value);

      // Write the data point to InfluxDB
      writeApi.writePoint(point);
    });
  }

  // Process tasks if available
  if (metricsPayload.tasks) {
    metricsPayload.tasks.forEach(task => {
      // Create a new data point for each task
      const point = new Point('device_tasks')
        .tag('device_id', device_id)
        .timestamp(timestamp)
        .intField(task.name, task.stack_free);

      // Write the data point to InfluxDB
      writeApi.writePoint(point);
    });
  }

  // Flush data to InfluxDB
  writeApi.flush()
    .then(() => {
      // Send success response
      res.status(200).send('Metrics, heap, and tasks received');
    })
    .catch((error) => {
      // Log and send error response if writing to InfluxDB fails
      console.error('Error writing to InfluxDB', error);
      res.status(500).send('Internal Server Error');
    });
});

// Start the server
app.listen(port, () => {
  console.log(`Server is running on http://localhost:${port}`);
});
