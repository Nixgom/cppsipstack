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


### ���� ����Ʈ


