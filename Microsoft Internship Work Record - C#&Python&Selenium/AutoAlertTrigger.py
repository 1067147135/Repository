from selenium import webdriver
import time

driver1 = webdriver.Edge("C:\\Users\\Administrator\\Downloads\\edgedriver_win64\\msedgedriver.exe")
driver2 = webdriver.Edge("C:\\Users\\Administrator\\Downloads\\edgedriver_win64\\msedgedriver.exe")
driver1.get("https://endpoint.microsoft.com/?feature.CloudPCGraphVersion=testprodbeta_cpc_int&Microsoft_Azure_CloudPC=int&Microsoft_Azure_CloudPC_urlredirect=cloudpctest&Microsoft_Intune=selfhost&Microsoft_Intune_Apps=selfhost&Microsoft_Intune_Devices=selfhost&Microsoft_Intune_DeviceSettings=selfhost&Microsoft_Intune_Enrollment=selfhost&Microsoft_Intune_Workflows=selfhost#blade/Microsoft_Intune_Devices/DeviceSettingsMenuBlade/overview/mdmDeviceId/fb850dc2-b6eb-4eeb-8ce4-5a2fa339d496")
driver2.get("https://endpoint.microsoft.com/?feature.CloudPCGraphVersion=testprodbeta_cpc_int&Microsoft_Azure_CloudPC=int&Microsoft_Azure_CloudPC_urlredirect=cloudpctest&Microsoft_Intune=selfhost&Microsoft_Intune_Apps=selfhost&Microsoft_Intune_Devices=selfhost&Microsoft_Intune_DeviceSettings=selfhost&Microsoft_Intune_Enrollment=selfhost&Microsoft_Intune_Workflows=selfhost#blade/Microsoft_Intune_Devices/DeviceSettingsMenuBlade/overview/mdmDeviceId/fb850dc2-b6eb-4eeb-8ce4-5a2fa339d496")
time.sleep(5)

myAccount = driver1.find_element_by_id("_weave_e_21")
myAccount.click()
myAccount = driver2.find_element_by_id("_weave_e_21")
myAccount.click()
time.sleep(2)

logpage = driver1.find_element_by_id("mectrl_signInItem")
logpage.click()
logpage = driver2.find_element_by_id("mectrl_signInItem")
logpage.click()
time.sleep(4)

othorAccount = driver1.find_element_by_xpath("//*[@id=\"otherTile\"]")
# othorAccount = driver1.find_element_by_id("otherTile")
othorAccount.click()
othorAccount = driver2.find_element_by_xpath("//*[@id=\"otherTile\"]")
othorAccount.click()
time.sleep(2)


inputAccount = driver1.find_element_by_id("i0116")
inputAccount.send_keys("connie@rplusint01.onmicrosoft.com")
inputAccount = driver2.find_element_by_id("i0116")
inputAccount.send_keys("connie@rplusint01.onmicrosoft.com")

submit = driver1.find_element_by_id("idSIButton9")
submit.click()
submit = driver2.find_element_by_id("idSIButton9")
submit.click()
time.sleep(3)

password = driver1.find_element_by_id("i0118")
password.send_keys("L0ngEn0ughPassw0rdCPC")
password = driver2.find_element_by_id("i0118")
password.send_keys("L0ngEn0ughPassw0rdCPC")

submit2 = driver1.find_element_by_id("idSIButton9")
submit2.click()
submit2 = driver2.find_element_by_id("idSIButton9")
submit2.click()

for i in range(30):
    print(i)
    try:
        driver2.refresh()
        time.sleep(10)
        restore = driver2.find_element_by_xpath("//div[@title=\"Restore (preview)\"]")
        restore.click()
        time.sleep(5)
        select = driver2.find_element_by_xpath("//div[@aria-describedby=\"fxc-gc-row-footer_0_0\"]")
        select.click()
        selected = driver2.find_element_by_xpath("//div[@title=\"Select\"]")
        selected.click()
        time.sleep(1)
        confirm = driver2.find_element_by_xpath("//div[@title=\"Restore\"]")
        confirm.click()
    except:
        restart = driver1.find_element_by_xpath("//div[@title=\"Restart\"]")
        restart.click()
        time.sleep(1)
        yes = driver1.find_element_by_xpath("//div[@title=\"Yes\"]")
        yes.click()

# reprovision = driver1.find_element_by_xpath("//div[@title=\"Reprovision\"]")
# reprovision.click()
# time.sleep(1)

time.sleep(100)

driver1.close()
driver2.close()

# "ActionType: {Monitor.Dimension.ActionType}, Environment: {Monitor.Dimension.Custom_Environment}, Cloud: {Monitor.Dimension.Tenant}, Start: {Monitor.DataStartTime}, End: {Monitor.DataEndTime}"
# 321739830