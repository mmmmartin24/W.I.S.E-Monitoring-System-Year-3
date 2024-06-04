import csv
import random

# Define the limits
temp_range = (20, 30)
humidity_range = (50, 100)
pressure_range = (0, 10)

# Define the number of rows to generate
num_rows = 50

# Open the csv file in write mode
with open('data.csv', 'w', newline='') as file:
    writer = csv.writer(file)
    # Write the header
    writer.writerow(["temperature", "humidity", "air_pressure", "door"])
    
    # Generate the rows
    for _ in range(num_rows):
        temp = random.uniform(*temp_range)
        humidity = random.uniform(*humidity_range)
        pressure = random.uniform(*pressure_range)
        
        # Determine the door value based on the conditions
        door = 1 if temp > 26 and humidity > 65 and pressure < 2.5 else 0
        
        # Write the row to the csv file
        writer.writerow([temp, humidity, pressure, door])

# Print a success message
print("The CSV file 'data.csv' has been successfully generated.")

