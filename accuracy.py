from pynodered import node_red
import requests
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score
import numpy as np

df = pd.read_csv("data.csv")

features = df[['temperature','humidity','air_pressure']]
target = df['door']

X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.3, random_state=42)
model = LogisticRegression()
model.fit(X_train,y_train)

predictions = model.predict(X_test)
accuracy = accuracy_score(y_test, predictions)
print(accuracy)

# Show prediction results
print("Predictions:")
for i, prediction in enumerate(predictions):
    print(f"Prediction for sample {i+1}: {prediction}")