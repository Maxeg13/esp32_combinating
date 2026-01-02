idf_component_register(SRCS "mag_qmc5883l.cpp"
                    INCLUDE_DIRS "."
					REQUIRES driver i2c_regs
					)