import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix
waste_types = ['cardboard','glass','metal','paper','plastic','trash']
y_true = ["cardboard", "cardboard", "cardboard", "plastic", "plastic", "trash"]
y_pred = ["glass", "cardboard", "cardboard", "plastic", "paper", "trash"]
cm=confusion_matrix(y_true, y_pred, labels=waste_types)
df_cm = pd.DataFrame(cm,waste_types,waste_types)
plt.figure(figsize=(10,8))
sns.heatmap(df_cm,annot=True,fmt="d",cmap="YlGnBu")
plt.show()