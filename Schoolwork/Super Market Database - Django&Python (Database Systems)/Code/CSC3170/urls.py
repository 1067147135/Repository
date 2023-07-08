"""CSC3170 URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path

from app import views

urlpatterns = [
    path('admin/', admin.site.urls),
    # www.xxx.com/index/ -> 函数
    # path('index/', views.index),
    # path('user/list/', views.user_list),
    # path('user/add/', views.user_add),
    # path('something/', views.something),
    # path('orm/', views.orm),
    path('load/', views.load),
    path('newload/', views.new_load),
    path('count/', views.return_top_products)
]
