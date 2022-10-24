from django.contrib import admin
from app.models import Customer, Product, Address, Order, customer_postal

admin.site.register([Customer, Product, Order, Address, customer_postal])
# admin.site.register(Product)
# admin.site.register(Order)
# admin.site.register(Address)
# admin.site.register(customer_postal)

# Register your models here.
