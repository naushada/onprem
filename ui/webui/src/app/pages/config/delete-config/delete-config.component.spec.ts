import { ComponentFixture, TestBed } from '@angular/core/testing';

import { DeleteConfigComponent } from './delete-config.component';

describe('DeleteConfigComponent', () => {
  let component: DeleteConfigComponent;
  let fixture: ComponentFixture<DeleteConfigComponent>;

  beforeEach(() => {
    TestBed.configureTestingModule({
      declarations: [DeleteConfigComponent]
    });
    fixture = TestBed.createComponent(DeleteConfigComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
