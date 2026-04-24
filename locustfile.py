from locust import task, HttpUser, constant

class User(HttpUser):
    wait_time = constant(0)
    @task
    def test_index(self):
        self.client.get("/")

    @task
    def test_index_full(self):
        self.client.get("/index.html")

    @task
    def test_page2(self):
        self.client.get("/page2.html")

    # @task
    # def test_not_found(self):
    #     self.client.get("/not_found.html")