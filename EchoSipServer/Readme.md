# EchoSipServer

### ����

* C++ SIP stack / SipUserAgent �� �̿��� ��ȭ echo ���� ���� ������Ʈ

### ������ ����
  
* �̸��� : websearch@naver.com
* ��α� : http://blog.naver.com/websearch

### ���̼���

* �� ������Ʈ�� ���̼����� GPLv3 �̰� ����� ���̼����� websearch@naver.com ���� ������ �ּ���.

### ���۹��
EchoSipServer �� ���۹���� ������ �����ϴ�.

* A �� CallID#1 �� INVITE �� EchoSipServer �� �����ϸ� EchoSipServer �� CallID#2 �� INVITE �� A ���� �����Ѵ�.
* A �� CallID#2 �� INVITE �� ���� 200 OK �� EchoSipServer �� �����ϸ� EchoSipServer �� CallID#1 �� INVITE �� ���� 200 OK �� A ���� �����Ѵ�.
* A �� CallID#1 �� BYE �� EchoSipServer �� �����ϸ� EchoSipServer �� CallID#2 �� BYE �� A ���� �����Ѵ�.

![Alt text](https://postfiles.pstatic.net/MjAxOTA0MDVfMTA2/MDAxNTU0NDQxNjkwOTM0.O96v0k7UB2hTKt5QhSDyPCjBsBHVZ5r4WEKXtehCSL0g.Q5sI7OJnWezwMPSIuIa1dD9_--KQrn_heEdVMnqX-JUg.PNG.websearch/EchoSipServer.png?type=w773)

### ���� ����Ʈ

* https://blog.naver.com/websearch/221506237184

