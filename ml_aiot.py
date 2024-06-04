from pynodered import node_red
import requests
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score
import numpy as np

df = pd.read_csv("data.csv")

@node_red(category="pyfunc-procs")
def check(node,msg):
    features = df[['temperature','humidity','air_pressure']]
    target = df['door']
    X_train, X_test, y_train, y_test = train_test_split(features, target, test_size=0.2, random_state=42)
    model = LogisticRegression()
    model.fit(X_train,y_train)

    payload = np.array(msg['payload']).reshape(1, -1)
    prediction = model.predict(msg['payload'])

    msg['payload'] = int(prediction[0])

    return msg

