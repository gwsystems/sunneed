# power_mgmt_infra

We are creating a framework for tracking and managing the distribution of power consumption of individual processes in multi-tenant computing environments. In systems with uncertain reserves of power available, such as a computer powered by a solar battery, it is impossible to guarantee unlimited power to each tenant. The basis of this project is to make a toolset for tenants to run **power-constrained code** on the system, limited by a power budget described by `sunneed`.

# Devices

The code for each device is compiled into a shared library. That shared library is opened by `sunneed` during runtime.
