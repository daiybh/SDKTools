from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication

import smtplib

import zipfile

import io,os

import Emailconfig
def sendMail_189(subject,text,z,zip_name):
    # 输入Email地址和口令:
    from_addr = Emailconfig.from_addr
    password = Emailconfig.password
    # 输入收件人地址:
    to_addr = Emailconfig.to_addr
    # 输入SMTP服务器地址:
    smtp_server = Emailconfig.smtp_server

    import time
    content='proect:\n{0}\n\n{1} \n\n\n\n{2}'.format(zip_name,text,time.asctime())
    textApart = MIMEText(content)

    zipApart=MIMEApplication(z.getvalue())
    zipApart.add_header('Content-Disposition','attachment',filename=zip_name+'.zip')

    m= MIMEMultipart()

    m.attach(textApart)
    m.attach(zipApart)

    m['Subject']='proect:{0}'.format(subject)

    print('smtp_server',smtp_server)
    server = smtplib.SMTP(smtp_server, 25) # SMTP协议默认端口是25
    #server.set_debuglevel(1)
    server.login(from_addr, password)
    server.sendmail(from_addr, to_addr.split(','), m.as_string())
    server.quit()



in_memory_zip = io.BytesIO()
z = zipfile.ZipFile(in_memory_zip,'w',zipfile.ZIP_DEFLATED)
flist=[r'sdktools.exe']
for a in flist:
    z.write(r'build/'+a,a)

z.close()



from git import Repo

repo=Repo('.')
headcommit = repo.head.commit

mailText='{0}\n{1}\n{2}'.format(repo.head.reference,repo.head.commit,headcommit.message)

print(mailText)

folder_path1 = os.getcwd()
folder_path2 = os.getcwd().replace('\\','/')
vl = folder_path1.split('\\')
print(vl[-1])


sendMail_189(vl[-1]+" "+headcommit.message,mailText,in_memory_zip,vl[-1])





