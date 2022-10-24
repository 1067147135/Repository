from email.policy import default
from tkinter import CASCADE
from django.db import models

# Create your models here.

"""
create table app_userinfo(
    id bigint auto_increment primary key
    PostalCode int,
    Segment = varchar(32)
    ...
)
"""

class Customer(models.Model):
    CustomerID = models.CharField(max_length=32, primary_key=True, unique=True, null=False)
    CustomerName = models.CharField(max_length=32, null=False)
    Segment = models.CharField(max_length=32, null=False)

class Product(models.Model):
    ProductID = models.CharField(max_length=32, primary_key=True, unique=True, null=False)
    Category = models.CharField(max_length=32, null=False)
    SubCategory = models.CharField(max_length=32, null=False)
    ProductName = models.CharField(max_length=128, null=False)

class Order(models.Model):
    OrderID = models.CharField(max_length=32, null=False)
    CustomerID = models.ForeignKey('Customer', on_delete=models.CASCADE)
    ProductID = models.ForeignKey('Product', on_delete=models.CASCADE)
    OrderDate = models.DateField(null=False)
    ShipDate = models.DateField()
    ShipMode = models.CharField(max_length=32)
    Sales = models.FloatField(null=False)
    Quantity = models.IntegerField(null=False)
    Discount = models.FloatField(null=False)
    Profit = models.FloatField(null=False)
    PostalCode = models.ForeignKey('Address', on_delete=models.CASCADE)
    class Meta:
        unique_together = [["OrderID", "CustomerID", "ProductID"]]

class Address(models.Model):
    PostalCode = models.IntegerField(primary_key=True, unique=True, null=False)
    Country = models.CharField(max_length=32, null=False)
    City = models.CharField(max_length=32, null=False)
    State = models.CharField(max_length=32, null=False)
    Region = models.CharField(max_length=32, null=False)

class customer_postal(models.Model):
    CustomerID = models.ForeignKey('Customer', on_delete=models.CASCADE)
    PostalCode = models.ForeignKey('Address', on_delete=models.CASCADE)
    class Meta:
        unique_together = [["CustomerID", "PostalCode"]]






